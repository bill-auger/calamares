/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
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

#ifndef PACSTRAPCPPJOB_H
#define PACSTRAPCPPJOB_H

#define QSTRINGMAP QMultiMap<QString , QString>

#include <QDir>
#include <QObject>
#include <QTimerEvent>
#include <QVariantMap>

#include <CppJob.h>
#include "GlobalStorage.h"


class PacstrapCppJob : public Calamares::CppJob
{
  Q_OBJECT


public:

  explicit PacstrapCppJob(const char* job_name         , const char* status_msg           ,
                          qreal       job_weight = 1.0 , QObject*    parent     = nullptr ) ;
  virtual ~PacstrapCppJob() ;

  void                 setConfigurationMap(const QVariantMap& config)       override ;
  qreal                getJobWeight       ()                          const override ;
  QString              prettyName         ()                          const override ;
  QString              prettyStatusMessage()                          const override ;
  Calamares::JobResult exec               ()                                override ;

  static const QString DESKTOP_PACKAGES_KEY ;


protected:

  QVariantMap execWithProgress(QString command_line , qreal task_proportion = 0.0) ;
  int         execStatus      (QString command_line , qreal task_proportion = 0.0) ;
  QString     execOutput      (QString command_line , qreal task_proportion = 0.0) ;
  QString     execError       (QString command_line , qreal task_proportion = 0.0) ;

  virtual QString getPackageList       () = 0 ;
  virtual QString chrootExecPreInstall () = 0 ;
  virtual QString chrootExecPostInstall() = 0 ;

  QString                   jobName ;
  QString                   statusMsg ;
  qreal                     jobWeight ;
  Calamares::GlobalStorage* globalStorage ;
  QVariantMap               localStorage ;
  QString                   mountPoint ;
  QString                   targetDevice ;
  QString                   confFile ;
  QString                   packages ;
  QDir                      cacheDir ;
  QDir                      metedataDir ;
  qint16                    nPreviousPackages ;
  qint16                    nPendingPackages ;
  qreal                     progressPercent ;


private:

  static        QString              FindTargetDevice(const QVariantList& partitions) ;
  static inline Calamares::JobResult JobError        (QString error_msg) ;
  static inline Calamares::JobResult JobSuccess      () ;

  qint16 nPackagesInstalled() ;
  qreal  emitProgress      (qreal transient_percent) ;
  qreal  getTaskCompletion () ;


  /* constants */

public:

  static const QString BOOTLODER_PACKAGES_KEY ;
  static const QString KERNEL_PACKAGES_KEY ;
  static const QString UTILITIES_PACKAGES_KEY ;
  static const QString OPENRC_PACKAGES_KEY ;
  static const QString SYSTEMD_PACKAGES_KEY ;
  static const QString APPLICATIONS_PACKAGES_KEY ;
  static const QString MULTIMEDIA_PACKAGES_KEY ;
  static const QString NETWORK_PACKAGES_KEY ;
  static const QString LOOKANDFEEL_PACKAGES_KEY ;
  static const QString XSERVER_PACKAGES_KEY ;
  static const QString LXDE_PACKAGES_KEY ;
  static const QString MATE_PACKAGES_KEY ;


protected:

//   static const QString    MOUNTPOINT ;
  static const char*      BASE_JOB_NAME ;
  static const char*      GUI_JOB_NAME ;
  static const char*      BASE_STATUS_MSG ;
  static const char*      GUI_STATUS_MSG ;
  static const qreal      BASE_JOB_WEIGHT ;
  static const qreal      GUI_JOB_WEIGHT ;
  static const QSTRINGMAP LANGUAGE_PACKS ;


private:

  static const QString PACKAGES_CACHE_DIR_FMT ;
  static const QString PACKAGES_METADATA_DIR_FMT ;
  static const QString DEFAULT_CONF_FILENAME ;
  static const QString ONLINE_CONF_FILENAME ;
  static const QString OFFLINE_CONF_FILENAME ;
  static const qreal   PACMAN_SYNC_PROPORTION ;
  static const qreal   LIST_PACKAGES_PROPORTION ;
  static const qreal   CHROOT_TASK_PROPORTION ;
  static const QString SYSTEM_EXEC_FMT ;
  static const QString PACSTRAP_CLEANUP_CMD ;
  static const QString KEYRING_CMD ;
  static const QString MOUNT_FMT ;
  static const QString CHROOT_PREP_FMT ;
  static const QString DB_REFRESH_FMT ;
  static const QString LIST_PACKAGES_FMT ;
  static const QString PACSTRAP_FMT ;
  static const QString UMOUNT_FMT ;
  static const QString CONFIG_ERROR_MSG ;
  static const QString TARGET_ERROR_MSG ;
  static const QString CONFFILE_ERROR_MSG ;
  static const QString KEYRING_ERROR_MSG ;
  static const QString MOUNT_ERROR_MSG ;
  static const QString CHROOT_PREP_ERROR_MSG ;
  static const QString PACMAN_SYNC_ERROR_MSG ;
  static const QString PACSTRAP_ERROR_MSG ;
  static const QString UMOUNT_ERROR_MSG ;
  static const QString STATUS_KEY ;
  static const QString STDOUT_KEY ;
  static const QString STDERR_KEY ;
} ;


/* DEBUG */

#define DEBUG_TRACE_EXEC cDebug() << "[PACSTRAP]: exec()" \
  << " job_name="     << this->jobName                    \
  << " has_isorepo="  << has_isorepo                      \
  << " is_online="    << is_online                        \
  << " mountPoint="   << this->mountPoint                 \
  << " targetDevice=" << this->targetDevice               \
  << " confFile="     << this->confFile                   \
  << " n_packages="   << this->packages.count()           ;

#define DEBUG_TRACE_EXECWITHPROGRESS                                           \
  cDebug()           << "[PACSTRAP]: shell command exited=" << command_line ;  \
  cDebug()           << "[PACSTRAP]: status="               << status ;        \
  cDebug(LOGVERBOSE) << "[PACSTRAP]: stdout='"              << stdout << "'" ; \
  cDebug(LOGVERBOSE) << "[PACSTRAP]: stderr='"              << stderr << "'"   ;

#define DEBUG_TRACE_FINDTARGETDEVICE if (!target_device.isEmpty())    \
  cDebug() << "[PACSTRAP]: mounting target_device: " << target_device ;

#define DEBUG_TRACE_EMITPROGRESS cDebug(LOGVERBOSE) << "[PACSTRAP]: " << \
  "this->progressPercent=" << this->progressPercent                   << \
  " transient_percent="    << transient_percent                       << \
  " emmitting="            << progress_percent                           ;

#define DEBUG_TRACE_GETTASKCOMPLETION cDebug(LOGVERBOSE) << "[PACSTRAP]: " << \
  "this->nPreviousPackages=" << this->nPreviousPackages                    << \
  " nPackagesInstalled()="   << nPackagesInstalled()                       << \
  "\n                          "                                           << \
  "n_new_packages="          << (int)n_new_packages                        << \
  " this->nPendingPackages=" << this->nPendingPackages                     << \
  " completion_percent="     << completion_percent                            ;

#ifndef QT_NO_DEBUG
#  define DEBUG_TRACE_BASEPACKAGES                       \
  printf("[PACSTRAP-GUI]: installing initsystem: %s\n" , \
         init_key.toStdString().c_str() )                ;
#  define DEBUG_TRACE_DESKTOPPACKAGES                                        \
  printf("[PACSTRAP-GUI]: installing initsystem: %s default_desktop: %s" ,   \
         init_key   .toStdString().c_str()                               ,   \
         desktop_key.toStdString().c_str()                               ) ; \
  printf("language_packs: %s for locale: %s\n"                         ,     \
         LANGUAGE_PACKS.values(locale).join(' ').toStdString().c_str() ,     \
         locale                                 .toStdString().c_str() )     ;
#else // QT_NO_DEBUG
#  define DEBUG_TRACE_BASEPACKAGES ;
#  define DEBUG_TRACE_DESKTOPPACKAGES ;
#endif // QT_NO_DEBUG


#endif // PACSTRAPCPPJOB_H
