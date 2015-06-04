// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2014 Red Hat
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */


#ifndef MDS_AUTH_CAPS_H
#define MDS_AUTH_CAPS_H

#include <vector>
#include <string>
#include <sstream>

// unix-style capabilities
enum {
  MAY_READ = 1,
  MAY_WRITE = 2,
  MAY_EXECUTE = 4,
};

// what we can do
struct MDSCapSpec {
  bool read;
  bool write;
  bool any;

  MDSCapSpec() : read(false), write(false), any(false) {}
  MDSCapSpec(bool r_, bool w_, bool a_) : read(r_), write(w_), any(a_) {}

  bool allow_all() const {
    return any;
  }
  bool allows(bool r, bool w) const {
    if (any)
      return true;
    if (r && !read)
      return false;
    if (w && !write)
      return false;
    return true;
  }
};

// conditions before we are allowed to do it
struct MDSCapMatch {
  static const int MDS_AUTH_UID_ANY = -1;
  static const std::string MDS_AUTH_PATH_ROOT;

  int uid;           // Require UID to be equal to this, if !=MDS_AUTH_UID_ANY
  std::vector<int> gids;  // Use these GIDs
  std::string path;  // Require path to be child of this (may be "/" for any)

  MDSCapMatch() : uid(MDS_AUTH_UID_ANY), path(MDS_AUTH_PATH_ROOT) {}
  MDSCapMatch(int uid_, std::vector<int>& gids_)
    : uid(uid_), gids(gids_), path(MDS_AUTH_PATH_ROOT) {}
  MDSCapMatch(std::string path_) : uid(MDS_AUTH_UID_ANY), path(path_) {}
  MDSCapMatch(std::string path_, int uid_, std::vector<int>& gids_)
    : uid(uid_), gids(gids_), path(path_) {}
  
  bool is_match_all() const
  {
    return uid == MDS_AUTH_UID_ANY && path == "/";
  }
  bool match(const std::string &target_path, const int target_uid) const;
};

struct MDSCapGrant {
  MDSCapSpec spec;
  MDSCapMatch match;

  MDSCapGrant(const MDSCapSpec &spec_, const MDSCapMatch &match_)
    : spec(spec_), match(match_) {}
  MDSCapGrant() {}
};

class MDSAuthCaps
{
  std::vector<MDSCapGrant> grants;

public:
  MDSAuthCaps() {}
  MDSAuthCaps(const std::vector<MDSCapGrant> &grants_) : grants(grants_) {}

  void set_allow_all();
  bool parse(const std::string &str, std::ostream *err);

  bool allow_all() const;
  bool is_capable(const std::string &path, int uid, unsigned mask) const;

  friend std::ostream &operator<<(std::ostream &out, const MDSAuthCaps &cap);
};


std::ostream &operator<<(std::ostream &out, const MDSCapMatch &match);
std::ostream &operator<<(std::ostream &out, const MDSCapSpec &spec);
std::ostream &operator<<(std::ostream &out, const MDSCapGrant &grant);
std::ostream &operator<<(std::ostream &out, const MDSAuthCaps &cap);

#endif // MDS_AUTH_CAPS_H
