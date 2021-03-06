/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2019, Adriaan de Groot <groot@kde.org>
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

#include "InitramfsJob.h"

#include "utils/CalamaresUtilsSystem.h"
#include "utils/Logger.h"
#include "utils/Variant.h"

InitramfsJob::InitramfsJob( QObject* parent )
    : Calamares::CppJob( parent )
{
}

InitramfsJob::~InitramfsJob() {}


QString
InitramfsJob::prettyName() const
{
    return tr( "Creating initramfs." );
}


Calamares::JobResult
InitramfsJob::exec()
{
    cDebug() << "Updating initramfs with kernel" << m_kernel;
    auto r = CalamaresUtils::System::instance()->targetEnvCommand(
        { "update-initramfs", "-k", m_kernel, "-c", "-t" }, QString(), QString(), 10 );
    return r.explainProcess( "update-initramfs", 10 );
}


void
InitramfsJob::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_kernel = CalamaresUtils::getString( configurationMap, "kernel" );
    if ( m_kernel.isEmpty() )
    {
        m_kernel = QStringLiteral( "all" );
    }
    else if ( m_kernel == "$uname" )
    {
        auto r = CalamaresUtils::System::runCommand(
            CalamaresUtils::System::RunLocation::RunInHost, { "/bin/uname", "-r" }, QString(), QString(), 3 );
        if ( r.getExitCode() == 0 )
        {
            m_kernel = r.getOutput();
            cDebug() << "*initramfs* using running kernel" << m_kernel;
        }
        else
        {
            cWarning() << "*initramfs* could not determine running kernel, using 'all'." << Logger::Continuation
                       << r.getExitCode() << r.getOutput();
        }
    }
}

CALAMARES_PLUGIN_FACTORY_DEFINITION( InitramfsJobFactory, registerPlugin< InitramfsJob >(); )
