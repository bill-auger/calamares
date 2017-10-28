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

#include "JobQueue.h"
#include <PacstrapCppJob.h>


/* PacstrapCppJob private class variables */

const QString PacstrapCppJob::MOUNTPOINT            = "/tmp/pacstrap" ;
const QDir    PacstrapCppJob::PACKAGES_CACHE_DIR    = QDir(MOUNTPOINT + "/var/cache/pacman/pkg") ;
const QDir    PacstrapCppJob::PACKAGES_METADATA_DIR = QDir(MOUNTPOINT + "/var/lib/pacman/local") ;
const char*   PacstrapCppJob::BASE_JOB_NAME         = "Pacstrap Base C++ Job" ;
const char*   PacstrapCppJob::GUI_JOB_NAME          = "Pacstrap Desktop C++ Job" ;
const char*   PacstrapCppJob::BASE_STATUS_MSG       = "Installing root filesystem" ;
const char*   PacstrapCppJob::GUI_STATUS_MSG        = "Installing graphical desktop environment" ;
const qreal   PacstrapCppJob::BASE_JOB_WEIGHT       = 23.0 ; // progress-bar job weight (1.0 normal)
const qreal   PacstrapCppJob::GUI_JOB_WEIGHT        = 69.0 ; // progress-bar job weight (1.0 normal)
const QString PacstrapCppJob::ONLINE_CONF_FILENAME  = "/etc/pacman-online.conf" ;
const QString PacstrapCppJob::OFFLINE_CONF_FILENAME = "/etc/pacman-offline.conf" ;
const QString PacstrapCppJob::IS_ONLINE_KEY         = "hasInternet" ;
const QString PacstrapCppJob::TARGET_DEVICE_KEY     = "target-device" ;
const QString PacstrapCppJob::SYSTEM_EXEC_FMT       = "/bin/sh -c \"%1\"" ;
// const QString PacstrapCppJob::KEYRING_CMD           = "pacman -Sy --noconfirm parabola-keyring && \
//                                                        pacman-key --populate parabola          && \
//                                                        pacman-key --refresh-keys                  " ;
// const QString PacstrapCppJob::KEYRING_CMD           = "pacman -Sy --noconfirm parabola-keyring" ;
const QString PacstrapCppJob::MKDIR_FMT             = "mkdir %1 2> /dev/null" ;
const QString PacstrapCppJob::MOUNT_FMT             = "mount %1 %2" ;
const QString PacstrapCppJob::CHROOT_PREP_FMT       = "mkdir -m 0755 -p {%1,%2}" ;
const QString PacstrapCppJob::PACKAGES_SYNC_FMT     = "pacman --print --config %1 --root %2 -Sy" ;
const QString PacstrapCppJob::LIST_PACKAGES_FMT     = "pacman --print --config %1 --root %2 -S %3" ;
const QString PacstrapCppJob::UMOUNT_FMT            = "umount %1" ;
const QString PacstrapCppJob::CONFIG_ERROR_MSG      = "Invalid configuration map." ;
const QString PacstrapCppJob::TARGET_ERROR_MSG      = "Target device for root filesystem is unspecified." ;
const QString PacstrapCppJob::CONFFILE_ERROR_MSG    = "Pacman configuration not found: '%1'." ;


/* PacstrapCppJob public constructors/destructors */

PacstrapCppJob::PacstrapCppJob(QString job_name   , QString  status_msg ,
                               qreal   job_weight , QObject* parent     ) :
                               jobName  (job_name  ) , statusMsg        (status_msg) ,
                               jobWeight(job_weight) , Calamares::CppJob(parent    )
{
printf("PacstrapCppJob::PacstrapCppJob() '%s'\n" , jobName.toStdString().c_str()) ;

  this->globalStorage = Calamares::JobQueue::instance()->globalStorage() ;
  this->guiTimerId    = startTimer(1000) ;
  this->confFile      = QString("") ; // deferred to exec()
  this->packages      = QString("") ; // deferred to loadPackageList()
  this->nPackages     = 0 ;           // deferred to setNPackages()
}

PacstrapCppJob::~PacstrapCppJob() {
printf("PacstrapCppJob::~PacstrapCppJob() '%s'\n" , jobName.toStdString().c_str()) ;

  killTimer(this->guiTimerId) ;
}


/* PacstrapCppJob public getters/setters */

void PacstrapCppJob::setConfigurationMap(const QVariantMap& config) { this->localStorage = config ; }

qreal PacstrapCppJob::getJobWeight()                          const { return this->jobWeight ; }

QString PacstrapCppJob::prettyName()                          const { return this->jobName ; }

QString PacstrapCppJob::prettyStatusMessage()                 const { return this->statusMsg ; }


/* PacstrapCppJob public instance methods */

Calamares::JobResult PacstrapCppJob::exec()
{
  setTargetDevice() ; loadPackageList() ;

  bool    has_internet  = this->globalStorage->value(IS_ONLINE_KEY    ).toBool() ;
  QString target_device = this->globalStorage->value(TARGET_DEVICE_KEY).toString() ;
  this->confFile        = (has_internet) ? ONLINE_CONF_FILENAME : OFFLINE_CONF_FILENAME ;

  if (this->localStorage.empty()     ) return JobErrorRetval(CONFIG_ERROR_MSG) ;
  if (target_device.isEmpty()        ) return JobErrorRetval(TARGET_ERROR_MSG) ;
  if (!QFile(this->confFile).exists()) return JobErrorRetval(CONFFILE_ERROR_MSG.arg(this->confFile)) ;

//   QString keyring_cmd       = KEYRING_CMD ;
  QString mkdir_cmd         = MKDIR_FMT        .arg(MOUNTPOINT) ;
  QString mount_cmd         = MOUNT_FMT        .arg(target_device , MOUNTPOINT) ;
  QString chroot_init_cmd   = CHROOT_PREP_FMT  .arg(PACKAGES_CACHE_DIR   .absolutePath() ,
                                                    PACKAGES_METADATA_DIR.absolutePath() ) ;
  QString pacman_sync_cmd   = PACKAGES_SYNC_FMT.arg(this->confFile , MOUNTPOINT) ;
  QString list_packages_cmd = LIST_PACKAGES_FMT.arg(this->confFile , MOUNTPOINT , this->packages) ;
  QString umount_cmd        = UMOUNT_FMT       .arg(target_device) ;

//     ExecWithStatus(keyring_cmd) ;
  ExecWithStatus(mkdir_cmd      ) ;
  ExecWithStatus(mount_cmd      ) ;
  ExecWithStatus(chroot_init_cmd) ;
  ExecWithStatus(pacman_sync_cmd) ;
  if (!this->packages.isEmpty() && setNPackages(list_packages_cmd) > 0)
  {
// return Calamares::JobResult::error("just cause") ;

    QString exec_error_msg = chrootExec() ;
    if (!exec_error_msg.isEmpty()) return JobErrorRetval(exec_error_msg) ;
  }
  else emit progress(this->jobWeight) ;
  ExecWithStatus(umount_cmd) ;

  return JobSuccessRetval() ;
}


/* PacstrapCppJob protected class methods */

Calamares::JobResult PacstrapCppJob::JobErrorRetval(QString error_msg)
{
  return Calamares::JobResult::error(error_msg) ;
}

Calamares::JobResult PacstrapCppJob::JobSuccessRetval()
{
  return Calamares::JobResult::ok() ;
}

QString PacstrapCppJob::QListToString(const QVariantList& package_list)
{
  QStringList result ;
  for (const QVariant& package : package_list) result.append(package.toString()) ;

  return result.join(' ') ;
}

qint16 PacstrapCppJob::NPackagesInstalled()
{
// QProcess::execute(QString("/bin/sh -c \"ls %1 | wc\"").arg(PACKAGES_CACHE_DIR   .absolutePath())) ;
// QProcess::execute(QString("/bin/sh -c \"ls %1 | wc\"").arg(PACKAGES_METADATA_DIR.absolutePath())) ;
// int     n1 = PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() ;
// QString s1 = PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).join(",") ;
// int     n2 = PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ;
// QString s2 = PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).join(",") ;
// printf("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_CACHE_DIR=%d\n" , n1) ;
// printf("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_CACHE_DIR=%s\n" , s1) ;
// printf("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_METADATA_DIR=%d\n" , n2) ;
// printf("[PACSTRAPCPP]: nPackagesInstalled() PACKAGES_METADATA_DIR=%s\n" , s2) ;
// printf("[PACSTRAPCPP]: nPackagesInstalled() ret=%d\n" , ((n1 + n2) / 2)) ;

  return (PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() +
          PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ) / 2 ;
}

int PacstrapCppJob::ExecWithStatus(QString command_line)
{
  return QProcess::execute(QString(SYSTEM_EXEC_FMT).arg(command_line)) ;
}

QStringList PacstrapCppJob::ExecWithOutput(QString command_line)
{
  QProcess proc ; proc.start(command_line) ; proc.waitForFinished(-1) ;

  QString stdout = proc.readAllStandardOutput() ;
  QString stderr = proc.readAllStandardError() ;

// cDebug() << "[PACSTRAPCPP_DEBUG]: PacstrapCppJob::ExecWithOutput() stdout=" << stdout ;
// cDebug() << "[PACSTRAPCPP_DEBUG]: PacstrapCppJob::ExecWithOutput() stderr=" << stderr ;

  return (QStringList() << stdout << stderr) ;
}


/* PacstrapCppJob protected instance methods */

void PacstrapCppJob::setTargetDevice()
{
  QString      target_device = "" ;
  QVariantList partitions    = this->globalStorage->value("partitions").toList() ;

  // locate target device for root filesystem
  foreach (const QVariant& partition , partitions)
  {
QStringList result; for ( auto it = partition.toMap().constBegin(); it != partition.toMap().constEnd(); ++it ) result.append( it.key() + '=' + it.value().toString() );
printf("[PACSTRAPCPP]: partition=[%s]\n" , result.join(',').toStdString().c_str()) ;

    QVariantMap partition_map = partition.toMap() ;
    QString     device        = partition_map.value("device").toString();
    QString     fs            = partition_map.value("fs").toString();
    QString     mountpoint    = partition_map.value("mountPoint").toString();
    QString     uuid          = partition_map.value("uuid").toString() ;

    if (mountpoint == "/") target_device = device ;

if (mountpoint == "/") printf("[PACSTRAPCPP]: target_device=%s\n" , device.toStdString().c_str()) ;
  }

  this->globalStorage->insert(TARGET_DEVICE_KEY , target_device) ;
}

qint16 PacstrapCppJob::setNPackages(QString list_packages_cmd)
{
  QString new_packages   = ExecWithOutput(list_packages_cmd).first() ;
  qint16  n_new_packages = new_packages.count(QChar::LineFeed) ;
  this->nPackages        = NPackagesInstalled() + n_new_packages ;

  return this->nPackages ;
}

void PacstrapCppJob::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == this->guiTimerId) updateProgress() ;
}

void PacstrapCppJob::updateProgress()
{
  if (this->nPackages == 0) return ;

  unsigned int progress_percent = (NPackagesInstalled() * 100) / this->nPackages ;
//   qreal progress_percent = nPackagesInstalled() / this->nPackages ;

printf("\n[PACSTRAPCPP]: n_packages=%d\n" , (int)NPackagesInstalled()) ;
printf("[PACSTRAPCPP]: this->nPackages=%d\n" , (int)this->nPackages) ;
printf("[PACSTRAPCPP]: progress_percent=%d\n" , progress_percent) ;

//   emit progress(0.5) ;
//   emit progress(progress_percent) ;
//   progress(qreal(progress_percent)) ;
  progress(qreal(progress_percent) / 100.0) ;
}
