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
#include "utils/Logger.h"


/* PacstrapCppJob public class constants */

const QString PacstrapCppJob::BOOTLODER_PACKAGES_KEY    = "bootloader" ;
const QString PacstrapCppJob::KERNEL_PACKAGES_KEY       = "kernel" ;
const QString PacstrapCppJob::OPENRC_PACKAGES_KEY       = "openrc" ;
const QString PacstrapCppJob::SYSTEMD_PACKAGES_KEY      = "systemd" ;
const QString PacstrapCppJob::APPLICATIONS_PACKAGES_KEY = "applications" ;
const QString PacstrapCppJob::MULTIMEDIA_PACKAGES_KEY   = "multimedia" ;
const QString PacstrapCppJob::NETWORK_PACKAGES_KEY      = "network" ;
const QString PacstrapCppJob::LOOKANDFEEL_PACKAGES_KEY  = "look-and-feel" ;
const QString PacstrapCppJob::UTILITIES_PACKAGES_KEY    = "utilities" ;
const QString PacstrapCppJob::XSERVER_PACKAGES_KEY      = "x-server" ;
const QString PacstrapCppJob::LXDE_PACKAGES_KEY         = "lxde" ;
const QString PacstrapCppJob::MATE_PACKAGES_KEY         = "mate" ;


/* PacstrapCppJob protected class constants */

// const QString    PacstrapCppJob::MOUNTPOINT      = "/tmp/pacstrap" ;
const char*      PacstrapCppJob::BASE_JOB_NAME   = "Pacstrap Base C++ Job" ;
const char*      PacstrapCppJob::GUI_JOB_NAME    = "Pacstrap Desktop C++ Job" ;
const char*      PacstrapCppJob::BASE_STATUS_MSG = "Installing root filesystem" ;
const char*      PacstrapCppJob::GUI_STATUS_MSG  = "Installing graphical desktop environment" ;
const qreal      PacstrapCppJob::BASE_JOB_WEIGHT = 20.0 ; // progress-bar job weight (1.0 normal)
const qreal      PacstrapCppJob::GUI_JOB_WEIGHT  = 57.0 ; // progress-bar job weight (1.0 normal)
const QSTRINGMAP PacstrapCppJob::LANGUAGE_PACKS  = { {"eo"          , "iceweasel-l10n-eo"    } ,
                                                     {"es_ES.UTF-8" , "icedove-l10n-es-es"   } ,
                                                     {"es_ES.UTF-8" , "iceweasel-l10n-es-es" } ,
                                                     {"fr_FR.UTF-8" , "icedove-l10n-fr"      } ,
                                                     {"fr_FR.UTF-8" , "iceweasel-l10n-fr"    } ,
                                                     {"gl_ES.UTF-8" , "icedove-l10n-gl"      } ,
                                                     {"gl_ES.UTF-8" , "iceweasel-l10n-gl"    } ,
                                                     {"it_IT.UTF-8" , "icedove-l10n-it"      } ,
                                                     {"it_IT.UTF-8" , "iceweasel-l10n-it"    } ,
                                                     {"pt_BR.UTF-8" , "icedove-l10n-pt-br"   } ,
                                                     {"pt_BR.UTF-8" , "iceweasel-l10n-pt-br" } ,
                                                     {"pl_PL.UTF-8" , "icedove-l10n-pl"      } ,
                                                     {"pl_PL.UTF-8" , "iceweasel-l10n-pl"    } } ;


/* PacstrapCppJob private class constants */

// const QDir    PacstrapCppJob::PACKAGES_CACHE_DIR       = QDir(MOUNTPOINT + "/var/cache/pacman/pkg") ;
// const QDir    PacstrapCppJob::PACKAGES_METADATA_DIR    = QDir(MOUNTPOINT + "/var/lib/pacman/local") ;
const QString PacstrapCppJob::PACKAGES_CACHE_DIR_FMT    = "%1/var/cache/pacman/pkg" ;
const QString PacstrapCppJob::PACKAGES_METADATA_DIR_FMT = "%1/var/lib/pacman/local" ;
const QString PacstrapCppJob::DEFAULT_CONF_FILENAME     = "/etc/pacman.conf" ;
const QString PacstrapCppJob::ONLINE_CONF_FILENAME     = "/etc/pacman-online.conf" ;
const QString PacstrapCppJob::OFFLINE_CONF_FILENAME    = "/etc/pacman-offline.conf" ;
const qreal   PacstrapCppJob::PACMAN_SYNC_PROPORTION   = 0.05 ; // per task progress-bar proportion
const qreal   PacstrapCppJob::LIST_PACKAGES_PROPORTION = 0.05 ; // per task progress-bar proportion
const qreal   PacstrapCppJob::CHROOT_TASK_PROPORTION   = 0.9 ;  // per task progress-bar proportion
const QString PacstrapCppJob::SYSTEM_EXEC_FMT          = "/bin/sh -c \"%1\"" ;
const QString PacstrapCppJob::PACSTRAP_CLEANUP_CMD     = "umount %1/dev/pts %1/dev/shm %1/dev %1/proc %1/run %1/sys %1/tmp %1 2> /dev/null" ;
// const QString PacstrapCppJob::MOUNT_FMT                = "mkdir %2 2> /dev/null || true && mount %1 %2" ;
const QString PacstrapCppJob::CHROOT_PREP_FMT          = "mkdir -m 0755 -p {%1,%2}" ;
const QString PacstrapCppJob::DB_REFRESH_FMT           = "pacman -S --print --config %1 --root %2 --refresh" ;
const QString PacstrapCppJob::LIST_PACKAGES_FMT        = "pacman -S --print --config %1 --root %2 %3" ;
const QString PacstrapCppJob::PACSTRAP_FMT             = "pacstrap -C %1 %2 %3 --noprogressbar" ;
// const QString PacstrapCppJob::KEYRING_CMD              = "pacman -Sy --noconfirm parabola-keyring" ;
const QString PacstrapCppJob::KEYRING_CMD              = "pacman -Sy --noconfirm archlinux-keyring       \
                                                                                 archlinux32-keyring     \
                                                                                 archlinuxarm-keyring    \
                                                                                 parabola-keyring     && \
                                                          pacman-key --init                           && \
                                                          pacman-key --populate archlinux                \
                                                                                archlinux32              \
                                                                                archlinuxarm             \
                                                                                parabola              && \
                                                          pacman-key --refresh-keys                      " ;
// const QString PacstrapCppJob::UMOUNT_FMT               = "umount %1" ;
const QString PacstrapCppJob::CONFIG_ERROR_MSG         = "Invalid configuration map." ;
const QString PacstrapCppJob::TARGET_ERROR_MSG         = "Target device for root filesystem is unspecified." ;
const QString PacstrapCppJob::CONFFILE_ERROR_MSG       = "Pacman configuration not found: '%1'." ;
const QString PacstrapCppJob::KEYRING_ERROR_MSG        = "Failed to update the pacman keyring." ;
const QString PacstrapCppJob::MOUNT_ERROR_MSG          = "Failed to mount the pacstrap chroot." ;
const QString PacstrapCppJob::CHROOT_PREP_ERROR_MSG    = "Failed to prepare the pacstrap chroot." ;
const QString PacstrapCppJob::PACMAN_SYNC_ERROR_MSG    = "Failed to syncronize packages in the pacstrap chroot." ;
const QString PacstrapCppJob::PACSTRAP_ERROR_MSG       = "Failed to install packages in chroot. " ;
// const QString PacstrapCppJob::UMOUNT_ERROR_MSG         = "Failed to unmount the pacstrap chroot." ;
const QString PacstrapCppJob::STATUS_KEY               = QString("status") ;
const QString PacstrapCppJob::STDOUT_KEY               = QString("stdout") ;
const QString PacstrapCppJob::STDERR_KEY               = QString("stderr") ;


/* PacstrapCppJob public constructors/destructors */

PacstrapCppJob::PacstrapCppJob(const char* job_name   , const char* status_msg ,
                               qreal       job_weight , QObject*    parent     ) :
                               jobName  (tr(job_name)) , statusMsg        (tr(status_msg)) ,
                               jobWeight(job_weight  ) , Calamares::CppJob(parent        )
{
  this->globalStorage     = Calamares::JobQueue::instance()->globalStorage() ;
  this->localStorage      = QVariantMap() ; // deferred to setConfigurationMap()
  this->mountPoint        = QString("") ;   // deferred to exec()
  this->targetDevice      = QString("") ;   // deferred to exec()
  this->confFile          = QString("") ;   // deferred to exec()
  this->packages          = QString("") ;   // deferred to exec()
  this->cacheDir          = QDir("") ;      // deferred to exec()
  this->metedataDir       = QDir("") ;      // deferred to exec()
  this->nPreviousPackages = 0 ;             // deferred to exec()
  this->nPendingPackages  = 0 ;             // deferred to exec()
  this->progressPercent   = 0 ;
}

PacstrapCppJob::~PacstrapCppJob() {}


/* PacstrapCppJob public getters/setters */

void PacstrapCppJob::setConfigurationMap(const QVariantMap& config) { this->localStorage = config ; }

qreal PacstrapCppJob::getJobWeight()                          const { return this->jobWeight ; }

QString PacstrapCppJob::prettyName()                          const { return this->jobName ; }

QString PacstrapCppJob::prettyStatusMessage()                 const { return this->statusMsg ; }


/* PacstrapCppJob public instance methods */

Calamares::JobResult PacstrapCppJob::exec()
{
// globalStorage->insert(GS::INITSYSTEM_KEY , OPENRC_PACKAGES_KEY) ; // TODO: per user option via globalStorage
globalStorage->insert(GS::INITSYSTEM_KEY , SYSTEMD_PACKAGES_KEY) ; // TODO: per user option via globalStorage
globalStorage->insert(GS::DESKTOP_KEY    , LXDE_PACKAGES_KEY   ) ; // TODO: per user option via globalStorage
// globalStorage->insert(GS::DESKTOP_KEY    , MATE_PACKAGES_KEY   ) ; // TODO: per user option via globalStorage

  // cleanup from possibly aborted previous runs
//   Teardown() ;

  QVariantList partitions  = this->globalStorage->value(GS::PARTITIONS_KEY     ).toList() ;
  bool         has_isorepo = this->globalStorage->value(GS::HAS_ISOREPO_KEY    ).toBool() ;
  bool         is_online   = this->globalStorage->value(GS::IS_ONLINE_KEY      ).toBool() ;
  this->mountPoint         = this->globalStorage->value(GS::ROOT_MOUNTPOINT_KEY).toString() ;
  this->targetDevice       = FindTargetDevice(partitions) ;
  this->confFile           = (!has_isorepo) ? DEFAULT_CONF_FILENAME :
                             (is_online   ) ? ONLINE_CONF_FILENAME  : OFFLINE_CONF_FILENAME ;
  this->packages           = getPackageList() ;
  this->cacheDir           = QDir(PACKAGES_CACHE_DIR_FMT   .arg(this->mountPoint)) ;
  this->metedataDir        = QDir(PACKAGES_METADATA_DIR_FMT.arg(this->mountPoint)) ;
  QString cache_path       = this->cacheDir   .absolutePath() ;
  QString metadata_path    = this->metedataDir.absolutePath() ;

DEBUG_TRACE_EXEC

  if (this->localStorage.empty()     ) return JobError(CONFIG_ERROR_MSG) ;
  if (this->targetDevice.isEmpty()   ) return JobError(TARGET_ERROR_MSG) ;
  if (!QFile(this->confFile).exists()) return JobError(CONFFILE_ERROR_MSG.arg(this->confFile)) ;

//   QString keyring_cmd       = KEYRING_CMD ;
//   QString mount_cmd         = MOUNT_FMT        .arg(this->targetDevice , this->mountPoint) ;
  QString chroot_prep_cmd   = CHROOT_PREP_FMT  .arg(cache_path , metadata_path) ;
  QString pacman_sync_cmd   = DB_REFRESH_FMT   .arg(this->confFile , this->mountPoint) ;
  QString list_packages_cmd = LIST_PACKAGES_FMT.arg(this->confFile , this->mountPoint , this->packages) ;
//   QString umount_cmd        = UMOUNT_FMT       .arg(this->targetDevice) ;

//   if (!!execStatus(keyring_cmd    )) return JobError(KEYRING_ERROR_MSG) ;
//   if (!!execStatus(mount_cmd                               )) return JobError(MOUNT_ERROR_MSG      ) ;
  if (!!execStatus(chroot_prep_cmd                         )) return JobError(CHROOT_PREP_ERROR_MSG) ;
  if (!!execStatus(pacman_sync_cmd , PACMAN_SYNC_PROPORTION)) return JobError(PACMAN_SYNC_ERROR_MSG) ;

  if (!this->packages.isEmpty())
  {
    QVariantMap result       = execWithProgress(list_packages_cmd , LIST_PACKAGES_PROPORTION) ;
    int         status       = result.value(STATUS_KEY).toInt() ;
    QString     new_packages = result.value(STDOUT_KEY).toString() ;
    QString     stderr       = result.value(STDERR_KEY).toString() ;
    this->nPreviousPackages  = nPackagesInstalled() ;
    this->nPendingPackages   = new_packages.count(QChar::LineFeed) ;

    if (!!status) return JobError(stderr) ;

    if (this->nPendingPackages > 0)
    {
      QString     pacstrap_cmd    = PACSTRAP_FMT.arg(this->confFile , this->mountPoint , this->packages) ;
      QVariantMap pacstrap_result = execWithProgress(pacstrap_cmd , CHROOT_TASK_PROPORTION) ;
      int         pacstrap_status = pacstrap_result.value(STATUS_KEY).toInt() ;
      QString     pacstrap_error  = pacstrap_result.value(STDERR_KEY).toString().trimmed() ;

      if (pacstrap_error.contains("signature"))
      {
        this->progressPercent = emitProgress(-CHROOT_TASK_PROPORTION) ;

        if (!!execStatus(KEYRING_CMD , CHROOT_TASK_PROPORTION * 0.5))
          return JobError(KEYRING_ERROR_MSG) ;

        pacstrap_result = execWithProgress(pacstrap_cmd , CHROOT_TASK_PROPORTION * 0.5) ;
        pacstrap_status = pacstrap_result.value(STATUS_KEY).toInt() ;
        pacstrap_error  = pacstrap_result.value(STDERR_KEY).toString().trimmed() ;
      }
      if (!!pacstrap_status) return JobError(PACSTRAP_ERROR_MSG + pacstrap_error) ;

      QString exec_error_msg = chrootExec() ;

      if (exec_error_msg.isEmpty()) this->nPendingPackages = 0 ;
      else                          return JobError(exec_error_msg) ;
    }
  }

//   if (!execStatus(umount_cmd)) this->progressPercent = emitProgress(1.0) ;
//   else                         return JobError(UMOUNT_ERROR_MSG) ;
  this->progressPercent = emitProgress(1.0) ;

  return JobSuccess() ;
}


/* PacstrapCppJob protected instance methods */

QVariantMap PacstrapCppJob::execWithProgress(QString command_line , qreal task_proportion)
{
  QProcess    proc ;
  int         status ;
  QString     stdout = QString("") ;
  QString     stderr = QString("") ;
  QVariantMap retval ;

  cLog() << "[PACSTRAP]: executing shell command: " << command_line ;
  cLog() << "=================== [SHELL OUTPUT BEGIN] ===================" ;
  proc.start(QString(SYSTEM_EXEC_FMT).arg(command_line)) ; proc.waitForStarted(-1) ;
  while (proc.waitForFinished(250) || proc.state() != QProcess::NotRunning)
  {
    QString stdout_flush ; stdout += (stdout_flush = proc.readAllStandardOutput()) ;
    QString stderr_flush ; stderr += (stderr_flush = proc.readAllStandardError()) ; ;

    if (!stdout_flush.isEmpty()) printf("%s" , stdout_flush.toStdString().c_str()) ;
    if (!stderr_flush.isEmpty()) printf("%s" , stderr_flush.toStdString().c_str()) ;

    emitProgress(task_proportion * getTaskCompletion()) ;
  }

  this->progressPercent = emitProgress(task_proportion) ;
  cLog() << "==================== [SHELL OUTPUT END] ====================" ;

  status = (proc.exitStatus() == QProcess::NormalExit) ? proc.exitCode() : 255 ;

DEBUG_TRACE_EXECWITHPROGRESS

  retval.insert(STATUS_KEY , QVariant(status)) ;
  retval.insert(STDOUT_KEY , QVariant(stdout)) ;
  retval.insert(STDERR_KEY , QVariant(stderr)) ;

  return retval ;
}

int PacstrapCppJob::execStatus(QString command_line , qreal task_proportion)
{
  QVariantMap result = execWithProgress(command_line , task_proportion) ;
  int         status = result.value(STATUS_KEY).toInt() ;

  return status ;
}

QString PacstrapCppJob::execOutput(QString command_line , qreal task_proportion)
{
  QVariantMap result = execWithProgress(command_line , task_proportion) ;
  QString     stdout = result.value(STDOUT_KEY).toString().trimmed() ;

  return stdout ;
}

QString PacstrapCppJob::execError(QString command_line , qreal task_proportion)
{
  QVariantMap result = execWithProgress(command_line , task_proportion) ;
  QString     stderr = result.value(STDERR_KEY).toString().trimmed() ;

  return stderr ;
}


/* PacstrapCppJob private class methods */

QString PacstrapCppJob::FindTargetDevice(const QVariantList& partitions)
{
  QString target_device = QString("") ;

  // locate target device for root filesystem
  foreach (const QVariant& partition , partitions)
  {
    QVariantMap partition_map = partition.toMap() ;
    QString     device        = partition_map.value(GS::DEVICE_KEY    ).toString() ;
    QString     fs            = partition_map.value(GS::FS_KEY        ).toString() ;
    QString     mountpoint    = partition_map.value(GS::MOUNTPOINT_KEY).toString() ;
    QString     uuid          = partition_map.value(GS::UUID_KEY      ).toString() ;

    if (mountpoint == "/") target_device = device ;
  }

DEBUG_TRACE_FINDTARGETDEVICE

  return target_device ;
}

Calamares::JobResult PacstrapCppJob::JobError(QString error_msg)
{
//   Calamares::GlobalStorage* global_storage = Calamares::JobQueue::instance()->globalStorage() ;
//   QString                   mountpoint     = global_storage.value(GS::ROOT_MOUNTPOINT_KEY).toString() ;
//   QProcess::execute(SYSTEM_EXEC_FMT.arg(PACSTRAP_CLEANUP_CMD.arg(mountpoint))) ;

  return Calamares::JobResult::error(error_msg) ;
}

Calamares::JobResult PacstrapCppJob::JobSuccess()
{
  return Calamares::JobResult::ok() ;
}


/* PacstrapCppJob private instance methods */

qint16 PacstrapCppJob::nPackagesInstalled()
{
  int n_downloaded = this->cacheDir   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() ;
  int n_installed  = this->metedataDir.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ;

  return (n_downloaded + n_installed) / 2 ;
}

qreal PacstrapCppJob::emitProgress(qreal transient_percent)
{
  qreal progress_percent = qBound(0.0 , this->progressPercent + transient_percent , 1.0) ;

DEBUG_TRACE_EMITPROGRESS

  emit progress(progress_percent) ;

  return progress_percent ;
}

qreal PacstrapCppJob::getTaskCompletion()
{
  if (this->nPendingPackages == 0) return 0.0 ;

  qreal n_new_packages     = qreal(nPackagesInstalled() - this->nPreviousPackages) ;
  qreal completion_percent = qreal(n_new_packages       / this->nPendingPackages ) ;

DEBUG_TRACE_GETTASKCOMPLETION

  return completion_percent ;
}
