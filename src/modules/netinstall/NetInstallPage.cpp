/*
 *   Copyright 2016, Luca Giambonini <almack@chakraos.org>
 *   Copyright 2016, Lisa Vitolo     <shainer@chakraos.org>
 *   Copyright 2017, Kyle Robbertze  <krobbertze@gmail.com>
 *   Copyright 2017, Adriaan de Groot <groot@kde.org>
 *   Copyright 2017, Gabriel Craciunescu <crazy@frugalware.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#include "NetInstallPage.h"

#include "PackageModel.h"
#include "ViewManager.h"

#include "ui_page_netinst.h"
#include "GlobalStorage.h"
#include "JobQueue.h"
#include "utils/Logger.h"
#include "utils/Retranslator.h"
#include "utils/YamlUtils.h"

#include <QFile>
#include <QMap>
#include <QTextStream>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QHeaderView>
#include <QtDebug>
#include <QtGlobal>
#include <QWidget>
#include <QSignalMapper>

#include <yaml-cpp/yaml.h>

using CalamaresUtils::yamlToVariant;


const char NetInstallPage::WMDE_COMBO_LABEL_TEXT[] = "Select Graphical Environment";


NetInstallPage::NetInstallPage( QWidget* parent )
    : QWidget( parent )
    , ui( new Ui::Page_NetInst )
    , m_networkManager( this )
    , m_groups( nullptr )
{
    ui->setupUi( this );

    ui->wmDeLabel->setText( tr( WMDE_COMBO_LABEL_TEXT ) );
    ui->wmDeCombobox->setToolTip( tr( WMDE_COMBO_LABEL_TEXT ) );
}

bool
NetInstallPage::readGroups( const QByteArray& yamlData )
{
    try
    {
        YAML::Node groups = YAML::Load( yamlData.constData() );

        if ( !groups.IsSequence() )
            cDebug() << "WARNING: netinstall groups data does not form a sequence.";
        Q_ASSERT( groups.IsSequence() );
        m_groups = new PackageModel( groups );
        CALAMARES_RETRANSLATE(
            m_groups->setHeaderData( 0, Qt::Horizontal, tr( "Name" ) );
            m_groups->setHeaderData( 1, Qt::Horizontal, tr( "Description" ) ); )
        return true;

    }
    catch ( YAML::Exception& e )
    {
        CalamaresUtils::explainYamlException( e, yamlData, "netinstall groups data" );
        return false;
    }
}

void
NetInstallPage::dataIsHere( QNetworkReply* reply )
{
    // If m_required is *false* then we still say we're ready
    // even if the reply is corrupt or missing.
    if ( reply->error() != QNetworkReply::NoError )
    {
        cDebug() << "WARNING: unable to fetch netinstall package lists.";
        cDebug() << "  ..Netinstall reply error: " << reply->error();
        cDebug() << "  ..Request for url: " << reply->url().toString() << " failed with: " << reply->errorString();
        ui->netinst_status->setText( tr( "Network Installation. (Disabled: Unable to fetch package lists, check your network connection)" ) );
        emit checkReady( !m_required );
        return;
    }

    if ( !readGroups( reply->readAll() ) )
    {
        cDebug() << "WARNING: netinstall groups data was received, but invalid.";
        cDebug() << "  ..Url:     " <<  reply->url().toString();
        cDebug() << "  ..Headers: " <<  reply->rawHeaderList();
        ui->netinst_status->setText( tr( "Network Installation. (Disabled: Received invalid groups data)" ) );
        reply->deleteLater();
        emit checkReady( !m_required );
        return;
    }

    ui->groupswidget->setModel( m_groups );
    ui->groupswidget->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
    ui->groupswidget->header()->setSectionResizeMode( 1, QHeaderView::Stretch );

    reply->deleteLater();
    emit checkReady( true );
}

void
NetInstallPage::loadEnvironmentComboboxes(QVariantMap local_storage)
{
    QVariantMap desktops_data = local_storage.value( GS::DESKTOPS_KEY ).toMap();
    QVariantMap::iterator desktops_iter = desktops_data.begin();

    for ( ; desktops_iter != desktops_data.end(); ++desktops_iter )
    {
cLog() << "NetInstallPage::loadEnvironmentComboboxes() desktops_iter.key()=" << desktops_iter.key();

        QVariantMap desktops_data = desktops_iter.value().toMap();
        QVariant de_key = QVariant(desktops_iter.key());
        QIcon de_icon = QIcon(desktops_data.value( GS::DE_ICON_KEY ).toString());
        QString de_name = desktops_data.value( GS::DE_NAME_KEY ).toString();

        ui->wmDeCombobox->addItem(de_icon, de_name, de_key);

cLog() << "NetInstallPage::loadEnvironmentComboboxes() added de_icon=" <<
        desktops_data.value( GS::DE_ICON_KEY ).toString() <<
        " de_key=" << de_key << " de_name" << de_name;
    }
    ui->wmDeCombobox->setCurrentIndex( 0 );
}

PackageModel::PackageItemDataList
NetInstallPage::selectedPackages() const
{
    if ( m_groups )
        return m_groups->getPackages();
    else
    {
        cDebug() << "WARNING: no netinstall groups are available.";
        return PackageModel::PackageItemDataList();
    }
}

QString
NetInstallPage::getWmDeKey() const
{
  return ui->wmDeCombobox->itemData(ui->wmDeCombobox->currentIndex()).toString();
}

void
NetInstallPage::loadGroupList()
{
    QString confUrl(
        Calamares::JobQueue::instance()->globalStorage()->value(
            "groupsUrl" ).toString() );

    QNetworkRequest request;
    request.setUrl( QUrl( confUrl ) );
    // Follows all redirects except unsafe ones (https to http).
    request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );
    // Not everybody likes the default User Agent used by this class (looking at you,
    // sourceforge.net), so let's set a more descriptive one.
    request.setRawHeader( "User-Agent", "Mozilla/5.0 (compatible; Calamares)" );

    connect( &m_networkManager, &QNetworkAccessManager::finished,
             this, &NetInstallPage::dataIsHere );
    m_networkManager.get( request );
}

void
NetInstallPage::setRequired( bool b )
{
    m_required = b;
}


void
NetInstallPage::onActivate()
{
    ui->groupswidget->setFocus();
}
