//
// Created by sanhuazhang on 2019/05/02
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

#import <WCDB/Assertion.hpp>
#import <WCDB/WCTDatabase+FTS.h>
#import <WCDB/WCTDatabase+Private.h>

NSString* const WCTTokenizerSimple = @"simple";
NSString* const WCTTokenizerPorter = @"porter";
NSString* const WCTTokenizerICU = @"icu";
NSString* const WCTTokenizerUnicode61 = @"unicode61";
NSString* const WCTTokenizerOneOrBinary = @"wcdb_one_or_binary";
NSString* const WCTTokenizerLegacyOneOrBinary = @"WCDB";
NSString* const WCTTokenizerVerbatim = @"wcdb_verbatim";
NSString* const WCTTokenizerPinyin = @"wcdb_pinyin";

NSString* const WCTTokenizerParameter_NeedSymbol = @"need_symbol";
NSString* const WCTTokenizerParameter_ChineseTraditionalToSimplified = @"chinese_traditional_to_simplified";
NSString* const WCTTokenizerParameter_SkipStemming = @"skip_stemming";

NSString* const WCTModuleFTS3 = @"fts3";
NSString* const WCTModuleFTS4 = @"fts4";
NSString* const WCTModuleFTS5 = @"fts5";

NSString* const WCTAuxiliaryFunction_SubstringMatchInfo = @"substring_match_info";

static NSDictionary* g_pinyinDict = nil;

static std::nullptr_t initializeTokenizer()
{
    WCDB::Core::shared().registerTokenizer(WCTTokenizerOneOrBinary, WCDB::FTS3TokenizerModuleTemplate<WCTOneOrBinaryTokenizerInfo, WCTOneOrBinaryTokenizer>::specialize());
    WCDB::Core::shared().registerTokenizer(WCTTokenizerLegacyOneOrBinary, WCDB::FTS3TokenizerModuleTemplate<WCTOneOrBinaryTokenizerInfo, WCTOneOrBinaryTokenizer>::specialize());
    [WCTDatabase registerTokenizer:WCDB::FTS5TokenizerModuleTemplate<WCTOneOrBinaryTokenizer>::specializeWithContext(nullptr) named:WCTTokenizerVerbatim];
    [WCTDatabase registerTokenizer:WCDB::FTS5TokenizerModuleTemplate<WCTPinyinTokenizer>::specializeWithContext(nullptr) named:WCTTokenizerPinyin];
    return nullptr;
}

static std::nullptr_t initializeAuxiliaryFunction()
{
    [WCTDatabase registerAuxiliaryFunction:WCDB::FTS5AuxiliaryFunctionTemplate<WCDB::SubstringMatchInfo>::specializeWithContext(nullptr) named:WCTAuxiliaryFunction_SubstringMatchInfo];
    return nullptr;
}

@implementation WCTDatabase (FTS)

- (void)enableAutoMergeFTS5Index:(BOOL)enable
{
    WCDB::Core::shared().enableAutoMergeFTSIndex(_database, enable);
}

- (void)addTokenizer:(NSString*)tokenizerName
{
    WCDB_ONCE(initializeTokenizer());

    WCDB::StringView configName = WCDB::StringView::formatted("%s%s", WCDB::TokenizeConfigPrefix, tokenizerName.UTF8String);
    _database->setConfig(configName, WCDB::Core::shared().tokenizerConfig(tokenizerName), WCDB::Configs::Priority::Higher);
}

+ (void)registerTokenizer:(const WCDB::TokenizerModule&)module named:(NSString*)name
{
    WCTRemedialAssert(name.length > 0, "Module name can't be nil.", return;);
    WCDB::Core::shared().registerTokenizer(name, module);
}

+ (void)configPinYinDict:(NSDictionary<NSString*, NSArray<NSString*>*>*)pinyinDict
{
    WCTFTSTokenizerUtil::configPinyinDict(pinyinDict);
}

+ (void)configTraditionalChineseDict:(NSDictionary<NSString*, NSString*>*)traditionalChineseDict
{
    WCTFTSTokenizerUtil::configTraditionalChineseDict(traditionalChineseDict);
}

- (void)addAuxiliaryFunction:(NSString*)auxiliaryFunctionName
{
    WCDB_ONCE(initializeAuxiliaryFunction());

    WCDB::StringView configName = WCDB::StringView::formatted("%s%s", WCDB::AuxiliaryFunctionConfigPrefix, auxiliaryFunctionName.UTF8String);
    _database->setConfig(configName, WCDB::Core::shared().auxiliaryFunctionConfig(auxiliaryFunctionName), WCDB::Configs::Priority::Higher);
}

+ (void)registerAuxiliaryFunction:(const WCDB::FTS5AuxiliaryFunctionModule&)module named:(NSString*)name
{
    WCTRemedialAssert(name.length > 0, "Module name can't be nil.", return;);
    WCDB::Core::shared().registerAuxiliaryFunction(name, module);
}

@end