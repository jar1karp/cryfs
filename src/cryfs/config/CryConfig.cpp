#include "CryConfig.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <gitversion/VersionCompare.h>
#include <cpp-utils/random/Random.h>

namespace bf = boost::filesystem;

using boost::property_tree::ptree;
using boost::optional;
using boost::none;
using std::string;
using std::stringstream;
using cpputils::Data;
using gitversion::VersionCompare;
using cpputils::Random;

namespace cryfs {

CryConfig::CryConfig()
: _rootBlob(""), _encKey(""), _cipher(""), _version(""), _createdWithVersion(""), _blocksizeBytes(0), _filesystemId(FilesystemID::Null()), _exclusiveClientId(none), _hasVersionNumbers(true) {
}

CryConfig::CryConfig(CryConfig &&rhs)
: _rootBlob(std::move(rhs._rootBlob)), _encKey(std::move(rhs._encKey)), _cipher(std::move(rhs._cipher)), _version(std::move(rhs._version)), _createdWithVersion(std::move(rhs._createdWithVersion)), _blocksizeBytes(rhs._blocksizeBytes), _filesystemId(std::move(rhs._filesystemId)), _exclusiveClientId(std::move(rhs._exclusiveClientId)), _hasVersionNumbers(rhs._hasVersionNumbers) {
}

CryConfig CryConfig::load(const Data &data) {
  stringstream stream;
  data.StoreToStream(stream);
  ptree pt;
  read_json(stream, pt);

  CryConfig cfg;
  cfg._rootBlob = pt.get<string>("cryfs.rootblob");
  cfg._encKey = pt.get<string>("cryfs.key");
  cfg._cipher = pt.get<string>("cryfs.cipher");
  cfg._version = pt.get<string>("cryfs.version", "0.8"); // CryFS 0.8 didn't specify this field, so if the field doesn't exist, it's 0.8.
  cfg._createdWithVersion = pt.get<string>("cryfs.createdWithVersion", cfg._version); // In CryFS <= 0.9.2, we didn't have this field, but also didn't update cryfs.version, so we can use this field instead.
  cfg._blocksizeBytes = pt.get<uint64_t>("cryfs.blocksizeBytes", 32832); // CryFS <= 0.9.2 used a 32KB block size which was this physical block size.
  cfg._exclusiveClientId = pt.get_optional<uint32_t>("cryfs.exclusiveClientId");
#ifndef CRYFS_NO_COMPATIBILITY
  cfg._hasVersionNumbers = pt.get<bool>("cryfs.migrations.hasVersionNumbers", false);
#endif

  optional<string> filesystemIdOpt = pt.get_optional<string>("cryfs.filesystemId");
  if (filesystemIdOpt == none) {
    cfg._filesystemId = Random::PseudoRandom().getFixedSize<FilesystemID::BINARY_LENGTH>();
  } else {
    cfg._filesystemId = FilesystemID::FromString(*filesystemIdOpt);
  }

  return cfg;
}

Data CryConfig::save() const {
  ptree pt;

  pt.put<string>("cryfs.rootblob", _rootBlob);
  pt.put<string>("cryfs.key", _encKey);
  pt.put<string>("cryfs.cipher", _cipher);
  pt.put<string>("cryfs.version", _version);
  pt.put<string>("cryfs.createdWithVersion", _createdWithVersion);
  pt.put<uint64_t>("cryfs.blocksizeBytes", _blocksizeBytes);
  pt.put<string>("cryfs.filesystemId", _filesystemId.ToString());
  if (_exclusiveClientId != none) {
    pt.put("cryfs.exclusiveClientId", *_exclusiveClientId);
  }
#ifndef CRYFS_NO_COMPATIBILITY
  pt.put("cryfs.migrations.hasVersionNumbers", _hasVersionNumbers);
#endif

  stringstream stream;
  write_json(stream, pt);
  return Data::LoadFromStream(stream);
}

const std::string &CryConfig::RootBlob() const {
  return _rootBlob;
}

void CryConfig::SetRootBlob(const std::string &value) {
  _rootBlob = value;
}

const string &CryConfig::EncryptionKey() const {
  return _encKey;
}

void CryConfig::SetEncryptionKey(const std::string &value) {
  _encKey = value;
}

const std::string &CryConfig::Cipher() const {
  return _cipher;
};

void CryConfig::SetCipher(const std::string &value) {
  _cipher = value;
}

const std::string &CryConfig::Version() const {
  return _version;
}

void CryConfig::SetVersion(const std::string &value) {
  _version = value;
}

const std::string &CryConfig::CreatedWithVersion() const {
  return _createdWithVersion;
}

void CryConfig::SetCreatedWithVersion(const std::string &value) {
  _createdWithVersion = value;
}

uint64_t CryConfig::BlocksizeBytes() const {
  return _blocksizeBytes;
}

void CryConfig::SetBlocksizeBytes(uint64_t value) {
  _blocksizeBytes = value;
}

const CryConfig::FilesystemID &CryConfig::FilesystemId() const {
  return _filesystemId;
}

void CryConfig::SetFilesystemId(const FilesystemID &value) {
  _filesystemId = value;
}

optional<uint32_t> CryConfig::ExclusiveClientId() const {
  return _exclusiveClientId;
}

void CryConfig::SetExclusiveClientId(optional<uint32_t> value) {
  _exclusiveClientId = value;
}

#ifndef CRYFS_NO_COMPATIBILITY
bool CryConfig::HasVersionNumbers() const {
  return _hasVersionNumbers;
}

void CryConfig::SetHasVersionNumbers(bool value) {
  _hasVersionNumbers = value;
}
#endif

}
