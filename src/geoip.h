/*
 * Bittorrent Client using Qt4 and libtorrent.
 * Copyright (C) 2006  Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * Contact : chris@qbittorrent.org
 */

#ifndef GEOIP_H
#define GEOIP_H

#include <libtorrent/session.hpp>
#include <QString>
#include <QDir>
#include <QFile>
#include "misc.h"

using namespace libtorrent;

// TODO: Update from Internet
class GeoIP {
protected:
  static QString geoipFolder(bool embedded=false) {
    if(embedded)
      return ":/geoip/";
    return misc::qBittorrentPath()+"geoip"+QDir::separator();
  }

  static QString geoipDBpath(bool embedded=false) {
    return geoipFolder(embedded)+"GeoIP.dat";
  }

  static QString geoipVersionPath(bool embedded=false) {
    return geoipFolder(embedded)+"VERSION";
  }

  static int getDBVersion(bool embedded = false) {
    QFile vFile(geoipVersionPath(embedded));
    qDebug("Reading file at %s", geoipVersionPath(embedded).toLocal8Bit().data());
    if(vFile.exists() && vFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug("File exists and was opened");
      QByteArray v = vFile.readAll().trimmed();
      /*while(!v.isEmpty() && v[0] == '0') {
        v = v.mid(1);
      }*/
      qDebug("Read version: %s", v.data());
      bool ok = false;
      int version = v.toInt(&ok);
      qDebug("Read version %d (Error? %d)", version, (int) !ok);
      if(ok) return version;
    }
    return 0;
  }

  static void updateLocalDb() {
    if(getDBVersion(true) > getDBVersion(false)) { // Update required
      qDebug("A local Geoip database update is required, proceeding...");
      // Create geoip folder is necessary
      QDir gfolder(geoipFolder(false));
      if(!gfolder.exists()) {
        if(!gfolder.mkpath(geoipFolder(false))) return;
      }
      // Remove destination files
      if(QFile::exists(geoipDBpath(false)))
        QFile::remove(geoipDBpath(false));
      if(QFile::exists(geoipVersionPath(false)))
        QFile::remove(geoipVersionPath(false));
      // Copy from executable to hard disk
      QFile::copy(geoipDBpath(true), geoipDBpath(false));
      QFile::copy(geoipVersionPath(true), geoipVersionPath(false));
      qDebug("Local Geoip database was updated");
    }
  }

public:
  static void loadDatabase(session *s) {
    updateLocalDb();
    if(QFile::exists(geoipDBpath(false))) {
      qDebug("Loading GeoIP database from %s...", geoipDBpath(false).toLocal8Bit().data());
      if(!s->load_country_db(geoipDBpath(false).toLocal8Bit().data())) {
        std::cerr << "Failed to load Geoip Database at " << geoipDBpath(false).toLocal8Bit().data() << std::endl;
      }
    } else {
      qDebug("ERROR: Impossible to find local Geoip Database");
    }
  }
};

#endif // GEOIP_H