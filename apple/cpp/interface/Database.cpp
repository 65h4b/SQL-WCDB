//
// Created by qiuwenchen on 2022/8/3.
//

/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <WCDB/CipherConfig.hpp>
#include <WCDB/Core.hpp>
#include <WCDB/CustomConfig.hpp>
#include <WCDB/Database.hpp>
#include <WCDB/FileManager.hpp>
#include <WCDB/InnerDatabase.hpp>
#include <WCDB/Version.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

namespace WCDB {

Database::Database(const UnsafeStringView& path)
{
    const char* resolvePath = realpath(path.data(), nullptr);
    if (resolvePath == nullptr && errno == ENOENT) {
        FileManager::createFile(path);
        resolvePath = realpath(path.data(), nullptr);
        FileManager::removeItem(path);
    }
    if (resolvePath != nullptr) {
        UnsafeStringView newPath = UnsafeStringView(resolvePath);
#ifdef __APPLE__
        /*
         /var is the symlink to /private/var.
         In most cases, realpath will return the path with the /var prefix,
         while in a small number of cases it will return the path with the /private/var prefix.
         In order to avoid the inconsistency of the path of the same file, remove the /private prefix of path here
         */
        if (newPath.hasPrefix("/private")) {
            newPath = UnsafeStringView(resolvePath + 8);
        }
#endif
        m_databaseHolder = Core::shared().getOrCreateDatabase(newPath);
        free((void*) resolvePath);
    } else {
        Error error;
        error.level = Error::Level::Error;
        error.setSystemCode(errno, Error::Code::IOError);
        error.infos.insert_or_assign(ErrorStringKeyPath, path);
        Notifier::shared().notify(error);
        m_databaseHolder = Core::shared().getOrCreateDatabase(path);
    }
    m_innerDatabase = m_databaseHolder.get();
}

Database::Database(const Database&) = default;

Database& Database::operator=(const Database&) = default;

Database::~Database() = default;

Database::Database(InnerDatabase* database) : m_innerDatabase(database)
{
}

RecyclableHandle Database::getHandleHolder()
{
    return m_databaseHolder->getHandle();
}

void Database::setTag(const long& tag)
{
    m_innerDatabase->setTag(tag);
}

long Database::getTag() const
{
    return m_innerDatabase->getTag();
}

const StringView& Database::getPath() const
{
    return m_innerDatabase->getPath();
}

Handle Database::getHandle()
{
    return Handle(m_databaseHolder);
}

bool Database::execute(Statement statement)
{
    return m_innerDatabase->execute(statement);
}

bool Database::canOpen() const
{
    return m_innerDatabase->canOpen();
}

bool Database::isOpened() const
{
    return m_innerDatabase->isOpened();
}

void Database::close(const Database::ClosedCallback& onClosed)
{
    m_innerDatabase->close(onClosed);
}

void Database::blockade()
{
    m_innerDatabase->blockade();
}

bool Database::isBlockaded() const
{
    return m_innerDatabase->isBlockaded();
}

void Database::unblockade()
{
    m_innerDatabase->unblockade();
}

bool Database::truncateCheckpoint()
{
    return m_innerDatabase->checkpoint(false, InnerDatabase::CheckPointMode::Truncate);
}

bool Database::passiveCheckpoint()
{
    return m_innerDatabase->checkpoint(false, InnerDatabase::CheckPointMode::Passive);
}

void Database::globalTraceError(Database::ErrorNotification trace)
{
    Core::shared().setNotificationWhenErrorTraced(trace);
}

void Database::globalTracePerformance(Database::PerformanceNotification trace)
{
    Core::shared().setNotificationWhenPerformanceGlobalTraced(trace);
}

void Database::tracePerformance(Database::PerformanceNotification trace)
{
    if (trace != nullptr) {
        m_innerDatabase->setConfig(
        PerformanceTraceConfigName,
        std::static_pointer_cast<Config>(std::make_shared<PerformanceTraceConfig>(trace)),
        Configs::Priority::Highest);
    } else {
        m_innerDatabase->removeConfig(PerformanceTraceConfigName);
    }
}

void Database::globalTraceSQL(Database::SQLNotification trace)
{
    Core::shared().setNotificationForSQLGLobalTraced(trace);
}

void Database::traceSQL(Database::SQLNotification trace)
{
    if (trace != nullptr) {
        m_innerDatabase->setConfig(
        SQLTraceConfigName,
        std::static_pointer_cast<Config>(std::make_shared<SQLTraceConfig>(trace)),
        Configs::Priority::Highest);
    } else {
        m_innerDatabase->removeConfig(SQLTraceConfigName);
    }
}

bool Database::removeFiles()
{
    return m_innerDatabase->removeFiles();
}

bool Database::moveFiles(const UnsafeStringView& directory)
{
    return m_innerDatabase->moveFiles(directory);
}

std::list<StringView> Database::getPaths() const
{
    return m_innerDatabase->getPaths();
}

std::optional<size_t> Database::getFilesSize() const
{
    return m_innerDatabase->getFilesSize();
}

void Database::enableAutoMergeFTS5Index(bool enable)
{
    Core::shared().enableAutoMergeFTSIndex(m_innerDatabase, enable);
}

void Database::addTokenizer(const UnsafeStringView& tokenize)
{
    StringView configName
    = StringView::formatted("%s%s", TokenizeConfigPrefix, tokenize.data());
    m_innerDatabase->setConfig(
    configName, Core::shared().tokenizerConfig(tokenize), Configs::Priority::Higher);
}

void Database::registerTokenizer(const UnsafeStringView& name, const TokenizerModule& module)
{
    Core::shared().registerTokenizer(name, module);
}

void Database::addAuxiliaryFunction(const UnsafeStringView& functionName)
{
    StringView configName = StringView::formatted(
    "%s%s", AuxiliaryFunctionConfigPrefix, functionName.data());
    m_innerDatabase->setConfig(configName,
                               Core::shared().auxiliaryFunctionConfig(functionName),
                               Configs::Priority::Higher);
}

void Database::registerAuxiliaryFunction(const UnsafeStringView& name,
                                         const FTS5AuxiliaryFunctionModule& module)
{
    Core::shared().registerAuxiliaryFunction(name, module);
}

void Database::purge()
{
    m_innerDatabase->purge();
}

void Database::purgeAll()
{
    Core::shared().purgeDatabasePool();
}

void Database::setNotificationWhenCorrupted(Database::CorruptionNotification onCorrupted)
{
    Core::shared().setNotificationWhenDatabaseCorrupted(
    getPath(), [onCorrupted](InnerDatabase* innerDatabase) {
        Database database = Database(innerDatabase);
        onCorrupted(database);
    });
}

bool Database::checkIfCorrupted()
{
    m_innerDatabase->checkIntegrity(false);
    return isAlreadyCorrupted();
}

bool Database::isAlreadyCorrupted()
{
    return Core::shared().isFileObservedCorrupted(getPath());
}

void Database::enableAutoBackup(bool enable)
{
    Core::shared().enableAutoBackup(m_innerDatabase, enable);
}

bool Database::backup()
{
    return m_innerDatabase->backup(false);
}

void Database::filterBackup(Database::BackupFilter tableShouldBeBackedUp)
{
    m_innerDatabase->filterBackup(tableShouldBeBackedUp);
}

bool Database::deposit()
{
    return m_innerDatabase->deposit();
}

bool Database::removeDeposited()
{
    return m_innerDatabase->removeDeposited();
}

bool Database::containsDeposited() const
{
    return m_innerDatabase->containsDeposited();
}

double Database::retrieve(Database::ProgressUpdateCallback onProgressUpdated)
{
    return m_innerDatabase->retrieve(onProgressUpdated);
}

void Database::setCipherKey(const UnsafeData& cipherKey, int cipherPageSize)
{
    if (cipherKey.size() > 0) {
        m_innerDatabase->setConfig(
        CipherConfigName,
        std::static_pointer_cast<Config>(std::make_shared<CipherConfig>(cipherKey, cipherPageSize)),
        Configs::Priority::Highest);
    } else {
        m_innerDatabase->removeConfig(CipherConfigName);
    }
}

void Database::setConfig(const UnsafeStringView& name,
                         Invocation invocation,
                         Invocation unInvocation,
                         Priority priority)
{
    m_innerDatabase->purge();
    CustomConfig::Invocation configInvocation
    = [invocation](InnerHandle* innerHandle) -> bool {
        Handle handle = Handle(innerHandle);
        return invocation(handle);
    };
    CustomConfig::Invocation configUninvocation = nullptr;
    if (unInvocation != nullptr) {
        configUninvocation = [unInvocation](InnerHandle* innerHandle) -> bool {
            Handle handle = Handle(innerHandle);
            return unInvocation(handle);
        };
    }
    m_innerDatabase->setConfig(name,
                               std::static_pointer_cast<Config>(std::make_shared<CustomConfig>(
                               configInvocation, configUninvocation)),
                               priority);
}

void Database::removeConfig(const UnsafeStringView& name)
{
    m_innerDatabase->removeConfig(name);
}

void Database::filterMigration(MigrationFilter filter)
{
    InnerDatabase::MigrationFilter callback = nullptr;
    if (filter != nullptr) {
        callback = [filter](MigrationUserInfo& userInfo) {
            MigrationInfo info;
            info.table = userInfo.getTable();
            info.database = userInfo.getDatabase();
            info.sourceTable = userInfo.getSourceTable();
            info.sourceDatabase = userInfo.getSourceDatabase();
            filter(info);
            if (info.sourceTable.length() > 0) {
                userInfo.setSource(info.sourceTable, info.sourceDatabase);
            }
        };
    }
    m_innerDatabase->filterMigration(callback);
}

bool Database::stepMigration()
{
    auto done = m_innerDatabase->stepMigration(false);
    return done.has_value();
}

void Database::enableAutoMigration(bool enable)
{
    Core::shared().enableAutoMigration(m_innerDatabase, enable);
}

void Database::setNotificationWhenMigrated(Database::MigratedCallback onMigrated)
{
    InnerDatabase::MigratedCallback callback = nullptr;
    if (onMigrated != nullptr) {
        callback = [onMigrated](InnerDatabase* innerDatabase,
                                const MigrationBaseInfo* baseInfo) {
            MigrationInfo info;
            info.table = baseInfo->getTable();
            info.database = baseInfo->getDatabase();
            info.sourceTable = baseInfo->getSourceTable();
            info.sourceDatabase = baseInfo->getSourceDatabase();
            Database database = Database(innerDatabase);
            onMigrated(database, info);
        };
    }
    m_innerDatabase->setNotificationWhenMigrated(callback);
}

bool Database::isMigrated() const
{
    return m_innerDatabase->isMigrated();
}

const StringView Database::getVersion()
{
    return StringView(WCDB_VERSION_STRING);
}

const StringView Database::getSourceId()
{
    return StringView(WCDB_SOURCE_ID);
}

} //namespace WCDB