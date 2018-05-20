/*
 *   Copyright 2016, Luca Giambonini <almack@chakraos.org>
 *   Copyright 2016, Lisa Vitolo <shainer@chakraos.org>
 *   Copyright 2017, Kyle Robbertze  <krobbertze@gmail.com>
 *   Copyright 2017, Adriaan de Groot <groot@kde.org>
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

#include "NetInstallViewStep.h"

#include "JobQueue.h"
#include "GlobalStorage.h"
#include "utils/Logger.h"

#include <QVariantList>

#include "NetInstallPage.h"

CALAMARES_PLUGIN_FACTORY_DEFINITION( NetInstallViewStepFactory, registerPlugin<NetInstallViewStep>(); )


const char NetInstallViewStep::PAGE_TITLE[] = "Packages";


NetInstallViewStep::NetInstallViewStep( QObject* parent )
    : Calamares::ViewStep( parent )
    , m_widget( new NetInstallPage() )
    , m_nextEnabled( false )
{
    emit nextStatusChanged( true );
    connect( m_widget, &NetInstallPage::checkReady,
             this, &NetInstallViewStep::nextIsReady );
}


NetInstallViewStep::~NetInstallViewStep()
{
    if ( m_widget && m_widget->parent() == nullptr )
        m_widget->deleteLater();
}


QString
NetInstallViewStep::prettyName() const
{
    return tr( PAGE_TITLE );
}


QString
NetInstallViewStep::prettyStatus() const
{
    return m_prettyStatus;
}


QWidget*
NetInstallViewStep::widget()
{
    return m_widget;
}


void
NetInstallViewStep::next()
{
    emit done();
}


void
NetInstallViewStep::back()
{}


bool
NetInstallViewStep::isNextEnabled() const
{
    return m_nextEnabled;
}


bool
NetInstallViewStep::isBackEnabled() const
{
    return true;
}


bool
NetInstallViewStep::isAtBeginning() const
{
    return true;
}


bool
NetInstallViewStep::isAtEnd() const
{
    return true;
}


QList< Calamares::job_ptr >
NetInstallViewStep::jobs() const
{
    return m_jobs;
}


void
NetInstallViewStep::onActivate()
{
    m_widget->onActivate();
}


void
NetInstallViewStep::onLeave()
{
    cDebug() << "Leaving netinstall, adding packages to be installed"
             << "to global storage";

    PackageModel::PackageItemDataList packages = m_widget->selectedPackages();
    QVariantList installPackages;
    QVariantList tryInstallPackages;
    QVariantList packageOperations;

    cDebug() << "Processing" << packages.length() << "packages from netinstall.";

    for ( auto package : packages )
    {
        QVariant details( package.packageName );
        // If it's a package with a pre- or post-script, replace
        // with the more complicated datastructure.
        if ( !package.preScript.isEmpty() || !package.postScript.isEmpty() )
        {
            QMap<QString, QVariant> sdetails;
            sdetails.insert( "pre-script", package.preScript );
            sdetails.insert( "package", package.packageName );
            sdetails.insert( "post-script", package.postScript );
            details = sdetails;
        }
        if ( package.isCritical )
            installPackages.append( details );
        else
            tryInstallPackages.append( details );
    }

    if ( !installPackages.empty() )
    {
        QMap<QString, QVariant> op;
        op.insert( "install", QVariant( installPackages ) );
        packageOperations.append( op );
        cDebug() << "  .." << installPackages.length() << "critical packages.";
    }
    if ( !tryInstallPackages.empty() )
    {
        QMap<QString, QVariant> op;
        op.insert( "try_install", QVariant( tryInstallPackages ) );
        packageOperations.append( op );
        cDebug() << "  .." << tryInstallPackages.length() << "non-critical packages.";
    }

    if ( !packageOperations.isEmpty() )
    {
        Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
        gs->insert( "packageOperations", QVariant( packageOperations ) );
    }

    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    gs->insert( GS::INITSYSTEM_KEY, m_widget->getInitSystem() ) ;
    gs->insert( GS::DESKTOP_KEY, m_widget->getWmDeKey() ) ;
    gs->insert( GS::IS_ONLINE_KEY, m_widget->getShouldNetInstall() );
}


void
NetInstallViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_widget->setRequired(
        configurationMap.contains( "required" ) &&
        configurationMap.value( "required" ).type() == QVariant::Bool &&
        configurationMap.value( "required" ).toBool() );

    if ( configurationMap.contains( "groupsUrl" ) &&
            configurationMap.value( "groupsUrl" ).type() == QVariant::String )
    {
cDebug() << "NetInstallViewStep::setConfigurationMap() groupsUrl=" << configurationMap.value( "groupsUrl" ).toString();

        // load package groups from netinstall.conf localStorage
        if ( configurationMap.contains( GS::PACKAGE_GROUPS_KEY ) &&
             configurationMap.value( GS::PACKAGE_GROUPS_KEY ).type() == QVariant::List &&
             configurationMap.value( "groupsUrl" ).toString() == PACKAGE_GROUPS_FILE )
        {
            // convert local storage packages lists to YAML
            QVariantList package_groups = configurationMap.value( GS::PACKAGE_GROUPS_KEY ).toList();
            YAML::Emitter package_groups_yaml;
            package_groups_yaml.SetOutputCharset(YAML::EscapeNonAscii);
            package_groups_yaml << YAML::BeginSeq;
            for ( int groups_n = 0; groups_n < package_groups.length(); ++groups_n )
            {
                QVariantMap package_group = package_groups.value(groups_n).toMap();
                std::string group_name = package_group["name"].toString().toStdString();
                std::string group_desc = package_group["description"].toString().toStdString();
                std::string group_critical = package_group["critical"].toString().toStdString();
                QStringList group_packages = package_group.value( "packages" ).toStringList();
                std::vector< std::string > packages;
                foreach ( const QString& package , group_packages )
                    packages.push_back( package.toStdString() );

cDebug() << "NetInstallViewStep::setConfigurationMap() package_group[name]=" << QString::fromStdString(group_name);
cDebug() << "NetInstallViewStep::setConfigurationMap() package_group[description]=" << QString::fromStdString(group_desc);
cDebug() << "NetInstallViewStep::setConfigurationMap() package_group[critical]=" << QString::fromStdString(group_critical);
cDebug() << "NetInstallViewStep::setConfigurationMap() package_group[packages]=" << group_packages;

                package_groups_yaml << YAML::BeginMap;
                package_groups_yaml << YAML::Key << "name"        << YAML::Value << group_name;
                package_groups_yaml << YAML::Key << "description" << YAML::Value << group_desc;
                package_groups_yaml << YAML::Key << "critical"    << YAML::Value << group_critical;
                package_groups_yaml << YAML::Key << "packages"    << YAML::Value << packages;
                package_groups_yaml << YAML::EndMap;
            }
            package_groups_yaml << YAML::EndSeq;

// DBG << "NetInstallViewStep::setConfigurationMap() err=" << QString(package_groups_yaml.GetLastError());

            QFile package_groups_file( QString( PACKAGE_GROUPS_FILE ).remove(0, 7) );
            if (package_groups_file.open( QIODevice::WriteOnly | QIODevice::Text ))
            {
                QTextStream out( &package_groups_file );
                out << package_groups_yaml.c_str(); // TODO:
            }
            cDebug() << "wrote package groups from netinstall.conf to" << PACKAGE_GROUPS_FILE;
        }
        else cDebug() << "no package groups defined in netinstall.conf - trying 'groupsUrl'";

        Calamares::JobQueue::instance()->globalStorage()->insert(
            "groupsUrl", configurationMap.value( "groupsUrl" ).toString() );
        m_widget->loadGroupList();
    }

    m_widget->populateComboboxes( configurationMap );
}

void
NetInstallViewStep::nextIsReady( bool b )
{
    m_nextEnabled = b;
    emit nextStatusChanged( b );
}
