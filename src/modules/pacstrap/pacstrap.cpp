/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014, Teo Mrnjavac <teo@kde.org> (original dummypython code)
 *   Copyright 2016, Kevin Kofler <kevin.kofler@chello.at>
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


#include <QProcess>
#include <QDateTime>
#include <QThread>

#include "CalamaresVersion.h"
#include "JobQueue.h"
// #include "GlobalStorage.h"

#include "pacstrap.h"
#include "utils/Logger.h"


/* PacStrapCppJob private class variables */

const QString PacStrapCppJob::MOUNTPOINT            = "/tmp/pacstrap" ;
const QDir    PacStrapCppJob::PACKAGES_CACHE_DIR    = QDir(MOUNTPOINT + "/var/cache/pacman/pkg") ;
const QDir    PacStrapCppJob::PACKAGES_METADATA_DIR = QDir(MOUNTPOINT + "/var/lib/pacman/local") ;
const char*   PacStrapCppJob::BASE_JOB_NAME         = "Pacstrap C++ Job" ;
const char*   PacStrapCppJob::GUI_JOB_NAME          = "Desktop C++ Job" ;
const char*   PacStrapCppJob::BASE_STATUS_MSG       = "Installing root filesystem" ;
const char*   PacStrapCppJob::GUI_STATUS_MSG        = "Installing graphical desktop environment" ;


/* PacStrapCppJob public instance methods */

PacStrapCppJob::PacStrapCppJob(QString   job_name    , QString           status_msg ,
                               qreal     job_weight  , QObject*          parent     ) :
                               jobName  (job_name  ) , statusMsg        (status_msg) ,
                               jobWeight(job_weight) , Calamares::CppJob(parent    )
{
cDebug() << "PacStrapCppJob::PacStrapCppJob() " << jobName ;

  this->globalStorage = Calamares::JobQueue::instance()->globalStorage() ;
  this->guiTimerId    = startTimer(1000) ;
  this->confFile      = QString("") ; // deferred to exec()
}

PacStrapCppJob::~PacStrapCppJob() {
cDebug() << "PacStrapCppJob::~PacStrapCppJob() " << jobName ;

  killTimer(this->guiTimerId) ;
}

qreal PacStrapCppJob::getJobWeight() const { return this->jobWeight ; }

QString PacStrapCppJob::prettyName() const { return this->jobName ; }

QString PacStrapCppJob::prettyStatusMessage() const { return this->statusMsg ; }

void PacStrapCppJob::setConfigurationMap(const QVariantMap& config) { this->config = config ; }

Calamares::JobResult PacStrapCppJob::exec()
{
// Calamares::GlobalStorage *gs = Calamares::JobQueue::instance()->globalStorage();
// foreach (const QString &key, gs->keys())
//   cDebug() << QString("[PACSTRAPCPP]: globalStorage[%1]=%2\n").arg( key, gs->value( key ).toString() );

// QProcess::execute( "echo SKIPPING parabola-prepare.desc" );

  setTargetDevice() ;

  bool    has_internet  = this->globalStorage->value("hasInternet"  ).toBool() ;
  QString target_device = this->globalStorage->value("target-device").toString() ;
  this->confFile        = (has_internet) ? "/etc/pacman-online.conf" : "/etc/pacman-offline.conf" ;

  if (this->config.empty()           ) return Calamares::JobResult::error("Invalid configuration map.") ;
  if (target_device.isEmpty()        ) return Calamares::JobResult::error("Target device for root filesystem is unspecified.") ;
  if (!QFile(this->confFile).exists()) return Calamares::JobResult::error(QString("Pacman configuration not found: '%1'.").arg(this->confFile)) ;

//     QString keyring_cmd = "/bin/sh -c \"pacman -Sy --noconfirm parabola-keyring && \
//                                         pacman-key --populate parabola          && \
//                                         pacman-key --refresh-keys                  \"";
//     QString keyring_cmd = "/bin/sh -c \"pacman -Sy --noconfirm parabola-keyring\"";
  QString mkdir_cmd  = QString("/bin/sh -c \"mkdir %1 2> /dev/null\"").arg(MOUNTPOINT) ;
  QString mount_cmd  = QString("/bin/sh -c \"mount %1 %2\"").arg(target_device , MOUNTPOINT) ;
  QString umount_cmd = QString("/bin/sh -c \"umount %1\"").arg(target_device) ;

//     QProcess::execute(keyring_cmd) ;
  QProcess::execute(mkdir_cmd) ;
  QProcess::execute(mount_cmd) ;
  QString exec_error_msg = chrootExec() ;
  if (!exec_error_msg.isEmpty()) return Calamares::JobResult::error(exec_error_msg) ;
  QProcess::execute(umount_cmd) ;

//     emit progress(1) ;
//   killTimer(this->guiTimerId) ;

  return Calamares::JobResult::ok() ;
}


/* PacStrapCppJob protected instance methods */

void PacStrapCppJob::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == this->guiTimerId) updateProgress() ;
}

void PacStrapCppJob::updateProgress()
{
  if (this->nPackages == 0) return ;

  unsigned int progress_percent = (nPackagesInstalled() * 100) / this->nPackages ;
//   qreal progress_percent = nPackagesInstalled() / this->nPackages ;

cDebug() << QString("") ;
cDebug() << QString("[PACSTRAPCPP]: n_packages=%1").arg(nPackagesInstalled()) ;
cDebug() << QString("[PACSTRAPCPP]: this->nPackages=%1").arg(this->nPackages) ;
cDebug() << QString("[PACSTRAPCPP]: progress_percent=%1").arg(progress_percent) ;
cDebug() << QString("") ;
//   emit progress(0.5) ;
//   emit progress(progress_percent) ;
//   progress(qreal(progress_percent)) ;
  progress(qreal(progress_percent) / 100.0) ;
}


/* PacStrapCppJob private class methods */

QString PacStrapCppJob::QListToString(const QVariantList& package_list)
{
  QStringList result ;
  for (const QVariant& package : package_list) result.append(package.toString()) ;

  return result.join(' ') ;
}

QStringList PacStrapCppJob::ExecWithOutput(QString command_line)
{
  QProcess proc ; proc.start(command_line) ; proc.waitForFinished(-1) ;

  QString stdout = proc.readAllStandardOutput() ;
  QString stderr = proc.readAllStandardError() ;

// cDebug() << "[PACSTRAPCPP_DEBUG]: PacStrapCppJob::ExecWithOutput() stdout=" << stdout ;
// cDebug() << "[PACSTRAPCPP_DEBUG]: PacStrapCppJob::ExecWithOutput() stderr=" << stderr ;

  return (QStringList() << stdout << stderr) ;
}


/* PacStrapCppJob private instance methods */

void PacStrapCppJob::setTargetDevice()
{
  QString      target_device = "" ;
  QVariantList partitions    = this->globalStorage->value("partitions").toList() ;

  // locate target device for root filesystem
  foreach (const QVariant& partition , partitions)
  {
QStringList result; for ( auto it = partition.toMap().constBegin(); it != partition.toMap().constEnd(); ++it ) result.append( it.key() + '=' + it.value().toString() );
cDebug() << QString("[PACSTRAPCPP]: partition=%1").arg('[' + result.join(',') + ']');

    QVariantMap partition_map = partition.toMap() ;
    QString     device        = partition_map.value("device").toString();
    QString     fs            = partition_map.value("fs").toString();
    QString     mountpoint    = partition_map.value("mountPoint").toString();
    QString     uuid          = partition_map.value("uuid").toString() ;

    if (mountpoint == "/") target_device = device ;

if (mountpoint == "/") cDebug() << QString("[PACSTRAPCPP]: target_device=%1").arg(device);
  }

  this->globalStorage->insert("target-device" , target_device) ;
}

qint16 PacStrapCppJob::nPackagesInstalled()
{
// QProcess::execute(QString("/bin/sh -c \"ls %1 | wc\"").arg(PACKAGES_CACHE_DIR   .absolutePath())) ;
// QProcess::execute(QString("/bin/sh -c \"ls %1 | wc\"").arg(PACKAGES_METADATA_DIR.absolutePath())) ;
// int     n1 = PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() ;
// QString s1 = PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).join(",") ;
// int     n2 = PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ;
// QString s2 = PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).join(",") ;
// cDebug() << QString("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_CACHE_DIR=%1").arg(n1) ;
// cDebug() << QString("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_CACHE_DIR=%1").arg(s1) ;
// cDebug() << QString("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_METADATA_DIR=%1").arg(n2) ;
// cDebug() << QString("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_METADATA_DIR=%1").arg(s2) ;
// cDebug() << QString("[PACSTRAPCPP]: nPackagesInstalled() ret=%1").arg((n1 + n2) / 2) ;

  return (PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() +
          PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ) / 2 ;
}

qint16 PacStrapCppJob::setNPackages(QString n_packages_cmd)
{
  QString new_packages   = ExecWithOutput(n_packages_cmd).first() ;
  qint16  n_new_packages = new_packages.count(QChar::LineFeed) ;
  this->nPackages        = nPackagesInstalled() + n_new_packages ;

  return this->nPackages ;
}


/* PacstrapCppJob public instance methods */

PacstrapCppJob::PacstrapCppJob(QObject* parent) : PacStrapCppJob(tr(BASE_JOB_NAME)   ,
                                                                 tr(BASE_STATUS_MSG) ,
                                                                 BASE_JOB_WEIGHT     ,
                                                                 parent              ) {}
// PacstrapCppJob::~PacstrapCppJob() { cDebug() << "PacstrapCppJob::~PacstrapCppJob() " << jobName ; }

/* PacstrapCppJob protected instance methods */

QString PacstrapCppJob::chrootExec()
{
  QString packages = QListToString(this->config.value("base"      ).toList() +
                                   this->config.value("bootloader").toList() +
                                   this->config.value("kernel"    ).toList() ) ;
  if (packages.isEmpty()) { emit progress(qreal(this->jobWeight)) ; return QString("") ; }

  QString chroot_init_cmd = QString("/bin/sh -c \"mkdir -m 0755 -p {%1,%2}\"").arg(PACKAGES_CACHE_DIR.absolutePath() , PACKAGES_METADATA_DIR.absolutePath()) ;
  QString pacman_sync_cmd = QString("/bin/sh -c \"pacman --print --config %1 --root %2 -Sy\"").arg(this->confFile , MOUNTPOINT) ;
  QString n_packages_cmd  = QString("/bin/sh -c \"pacman --print --config %1 --root %2 -S %3\"").arg(this->confFile , MOUNTPOINT , packages) ;
  QString pacstrap_cmd    = QString("/bin/sh -c \"pacstrap -C %1 %2 %3\"").arg(this->confFile , MOUNTPOINT , packages) ;
  QString grub_theme_cmd  = QString("/bin/sh -c \"sed -i 's|[#]GRUB_THEME=.*|GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt|' %1/etc/default/grub\"").arg(MOUNTPOINT) ;

QString grub_theme_kludge_cmd = QString("/bin/sh -c \"echo GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt >> %1/etc/default/grub\"").arg(MOUNTPOINT) ;
cDebug() << QString("[PACSTRAPCPP]: grub_theme_cmd=%1").arg(grub_theme_cmd);
// QProcess::execute( "/bin/sh -c \"ls /tmp/\"" );

    // boot-strap install root filesystem
//   this->guiTimerId = startTimer(1000) ; updateProgress() ;

//   #include <QTimer>
//   QTimer *timer = new QTimer(this);
//   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
//   timer->start(1000);

  QProcess::execute(chroot_init_cmd) ;
  QProcess::execute(pacman_sync_cmd) ;
  if (setNPackages(n_packages_cmd) == 0) return "No packages to install." ;
// return Calamares::JobResult::error("just cause") ;
  if (QProcess::execute(pacstrap_cmd)  ) return "PACSTRAP_FAIL" ;
//     m_status = tr( "Installing linux-libre kernel" ); emit progress( 5 );

cDebug() << QString("[PACSTRAPCPP]: grub_theme_cmd IN");  QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));
    QProcess::execute(grub_theme_cmd) ;
QProcess::execute(grub_theme_kludge_cmd) ;
cDebug() << QString("[PACSTRAPCPP]: grub_theme_cmd OUT"); QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));
//     emit progress(5) ;
//     QProcess::execute( kernel_cmd );

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapCppJobFactory , registerPlugin<PacstrapCppJob>() ;)
