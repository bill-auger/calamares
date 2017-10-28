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

  explicit PacstrapCppJob(QString job_name         , QString  status_msg           ,
                          qreal   job_weight = 1.0 , QObject* parent     = nullptr ) ;
  virtual ~PacstrapCppJob() ;

  void                 setConfigurationMap(const QVariantMap& config)       override ;
  qreal                getJobWeight       ()                          const override ;
  QString              prettyName         ()                          const override ;
  QString              prettyStatusMessage()                          const override ;
  Calamares::JobResult exec               ()                                override ;

  static const QString DESKTOP_PACKAGES_KEY ;


protected:

  static Calamares::JobResult JobErrorRetval    (QString error_msg) ;
  static Calamares::JobResult JobSuccessRetval  () ;
  static QString              QListToString     (const QVariantList& package_list) ;
  static QString              FindTargetDevice  (const QVariantList& partitions) ;
  static unsigned int         NPackagesInstalled() ;
  static QStringList          Exec              (QString command_line) ;
  static int                  ExecWithStatus    (QString command_line) ;
  static QString              ExecWithOutput    (QString command_line) ;
  static QString              ExecWithError     (QString command_line) ;

  virtual QString getPackageList() = 0 ;
  virtual QString chrootExec    () = 0 ;

  void                 timerEvent    (QTimerEvent* event) override ;
  void                 updateProgress() ;
  Calamares::JobResult runJob        () ;

  static const QString MOUNTPOINT ;
  static const char*   BASE_JOB_NAME ;
  static const char*   GUI_JOB_NAME ;
  static const char*   BASE_STATUS_MSG ;
  static const char*   GUI_STATUS_MSG ;
  static const qreal   BASE_JOB_WEIGHT ;
  static const qreal   GUI_JOB_WEIGHT ;
  static const QString BASE_PACKAGES_KEY ;
  static const QString BOOTLODER_PACKAGES_KEY ;
  static const QString KERNEL_PACKAGES_KEY ;
  static const QString APPLICATIONS_PACKAGES_KEY ;
  static const QString MULTIMEDIA_PACKAGES_KEY ;
  static const QString NETWORK_PACKAGES_KEY ;
  static const QString THEMES_PACKAGES_KEY ;
  static const QString UTILITIES_PACKAGES_KEY ;
  static const QString XSERVER_PACKAGES_KEY ;
  static const QString MATE_PACKAGES_KEY ;
  static const QString LXDE_PACKAGES_KEY ;
  static const QString PACSTRAP_FMT ;
  static const QString PACSTRAP_ERROR_MSG ;

  QString                   jobName ;
  QString                   statusMsg ;
  qreal                     jobWeight ;
  Calamares::GlobalStorage* globalStorage ;
  unsigned int              guiTimerId ;
  QString                   targetDevice ;
  QString                   confFile ;
  QString                   packages ;
  unsigned int              nPackages ;
  QVariantMap               localStorage ;


private:

  static const QDir    PACKAGES_CACHE_DIR ;
  static const QDir    PACKAGES_METADATA_DIR ;
  static const QString ONLINE_CONF_FILENAME ;
  static const QString OFFLINE_CONF_FILENAME ;
  static const QString IS_ONLINE_KEY ;
  static const QString PARTITIONS_KEY ;
  static const QString DEVICE_KEY ;
  static const QString FS_KEY ;
  static const QString MOUNTPOINT_KEY ;
  static const QString UUID_KEY ;
  static const QString SYSTEM_EXEC_FMT ;
//   static const QString KEYRING_CMD ;
  static const QString MOUNT_FMT ;
  static const QString CHROOT_PREP_FMT ;
  static const QString PACKAGES_SYNC_FMT ;
  static const QString LIST_PACKAGES_FMT ;
  static const QString UMOUNT_FMT ;
  static const QString CONFIG_ERROR_MSG ;
  static const QString TARGET_ERROR_MSG ;
  static const QString CONFFILE_ERROR_MSG ;
//   static const QString KEYRING_ERROR_MSG ;
  static const QString MOUNT_ERROR_MSG ;
  static const QString CHROOT_PREP_ERROR_MSG ;
  static const QString PACMAN_SYNC_ERROR_MSG ;
  static const QString UMOUNT_ERROR_MSG ;
} ;


#endif // PACSTRAPCPPJOB_H
