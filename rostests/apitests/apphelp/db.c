/*
 * Copyright 2012 Detlef Riekenberg
 * Copyright 2013 Mislav Blažević
 * Copyright 2015,2016 Mark Jansen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */


#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <shlwapi.h>
#include <winnt.h>
#ifdef __REACTOS__
#include <ntndk.h>
#else
#include <winternl.h>
#endif

#include <winerror.h>
#include <stdio.h>
#include <initguid.h>

#include "wine/test.h"

/* data.c */
void test_create_db_imp(const char* name);

#define test_create_db      (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : test_create_db_imp

typedef WORD TAG;
typedef DWORD TAGID;
typedef DWORD TAGREF;
typedef UINT64 QWORD;
typedef VOID* PDB;
typedef INT PATH_TYPE;
#define DOS_PATH 0

#define TAGID_NULL 0x0
#define TAGID_ROOT 0x0
#define _TAGID_ROOT 12


#define TAG_TYPE_MASK 0xF000

#define TAG_TYPE_NULL 0x1000
#define TAG_TYPE_BYTE 0x2000
#define TAG_TYPE_WORD 0x3000
#define TAG_TYPE_DWORD 0x4000
#define TAG_TYPE_QWORD 0x5000
#define TAG_TYPE_STRINGREF 0x6000
#define TAG_TYPE_LIST 0x7000
#define TAG_TYPE_STRING 0x8000
#define TAG_TYPE_BINARY 0x9000
#define TAG_NULL 0x0

#define TAG_INCLUDE (0x1 | TAG_TYPE_NULL)

#define TAG_MATCH_MODE (0x1 | TAG_TYPE_WORD)

#define TAG_SIZE (0x1 | TAG_TYPE_DWORD)
#define TAG_CHECKSUM (0x3 | TAG_TYPE_DWORD)
#define TAG_MODULE_TYPE (0x6 | TAG_TYPE_DWORD)
#define TAG_VERFILEOS (0x9 | TAG_TYPE_DWORD)
#define TAG_VERFILETYPE (0xA | TAG_TYPE_DWORD)
#define TAG_PE_CHECKSUM (0xB | TAG_TYPE_DWORD)
#define TAG_PROBLEMSEVERITY (0x10 | TAG_TYPE_DWORD)
#define TAG_HTMLHELPID (0x15 | TAG_TYPE_DWORD)
#define TAG_FLAGS (0x17 | TAG_TYPE_DWORD)
#define TAG_LAYER_TAGID (0x1A | TAG_TYPE_DWORD)
#define TAG_LINKER_VERSION (0x1C | TAG_TYPE_DWORD)
#define TAG_LINK_DATE (0x1D | TAG_TYPE_DWORD)
#define TAG_UPTO_LINK_DATE (0x1E | TAG_TYPE_DWORD)
#define TAG_APP_NAME_RC_ID (0x24 | TAG_TYPE_DWORD)
#define TAG_VENDOR_NAME_RC_ID (0x25 | TAG_TYPE_DWORD)
#define TAG_SUMMARY_MSG_RC_ID (0x26 | TAG_TYPE_DWORD)
#define TAG_OS_PLATFORM (0x23 | TAG_TYPE_DWORD)

#define TAG_TIME (0x1 | TAG_TYPE_QWORD)
#define TAG_BIN_FILE_VERSION (0x2 | TAG_TYPE_QWORD)
#define TAG_BIN_PRODUCT_VERSION (0x3 | TAG_TYPE_QWORD)
#define TAG_UPTO_BIN_PRODUCT_VERSION (0x6 | TAG_TYPE_QWORD)
#define TAG_UPTO_BIN_FILE_VERSION (0xD | TAG_TYPE_QWORD)
#define TAG_FLAG_LUA (0x10 | TAG_TYPE_QWORD)

#define TAG_DATABASE (0x1 | TAG_TYPE_LIST)
#define TAG_INEXCLUD (0x3 | TAG_TYPE_LIST)
#define TAG_EXE (0x7 | TAG_TYPE_LIST)
#define TAG_MATCHING_FILE (0x8 | TAG_TYPE_LIST)
#define TAG_SHIM_REF (0x9| TAG_TYPE_LIST)
#define TAG_LAYER (0xB | TAG_TYPE_LIST)
#define TAG_APPHELP (0xD | TAG_TYPE_LIST)
#define TAG_LINK (0xE | TAG_TYPE_LIST)
#define TAG_STRINGTABLE (0x801 | TAG_TYPE_LIST)

#define TAG_STRINGTABLE_ITEM (0x801 | TAG_TYPE_STRING)

#define TAG_NAME (0x1 | TAG_TYPE_STRINGREF)
#define TAG_MODULE (0x3 | TAG_TYPE_STRINGREF)
#define TAG_VENDOR (0x5 | TAG_TYPE_STRINGREF)
#define TAG_APP_NAME (0x6 | TAG_TYPE_STRINGREF)
#define TAG_COMMAND_LINE (0x8 | TAG_TYPE_STRINGREF)
#define TAG_COMPANY_NAME (0x9 | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_NAME (0x10 | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_VERSION (0x11 | TAG_TYPE_STRINGREF)
#define TAG_FILE_DESCRIPTION (0x12 | TAG_TYPE_STRINGREF)
#define TAG_FILE_VERSION (0x13 | TAG_TYPE_STRINGREF)
#define TAG_ORIGINAL_FILENAME (0x14 | TAG_TYPE_STRINGREF)
#define TAG_INTERNAL_NAME (0x15 | TAG_TYPE_STRINGREF)
#define TAG_LEGAL_COPYRIGHT (0x16 | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_DETAILS (0x18 | TAG_TYPE_STRINGREF)
#define TAG_LINK_URL (0x19 | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_TITLE (0x1B | TAG_TYPE_STRINGREF)

#define TAG_COMPILER_VERSION (0x22 | TAG_TYPE_STRINGREF)

#define TAG_GENERAL (0x2 | TAG_TYPE_NULL)

#define TAG_EXE_ID (0x4 | TAG_TYPE_BINARY)
#define TAG_DATA_BITS (0x5 | TAG_TYPE_BINARY)
#define TAG_DATABASE_ID (0x7 | TAG_TYPE_BINARY)



static HMODULE hdll;
static LPCWSTR (WINAPI *pSdbTagToString)(TAG);
static PDB (WINAPI *pSdbOpenDatabase)(LPCWSTR, PATH_TYPE);
static PDB (WINAPI *pSdbCreateDatabase)(LPCWSTR, PATH_TYPE);
static BOOL (WINAPI *pSdbGetDatabaseVersion)(LPCWSTR, PDWORD, PDWORD);
static void (WINAPI *pSdbCloseDatabase)(PDB);
static void (WINAPI *pSdbCloseDatabaseWrite)(PDB);
static TAG (WINAPI *pSdbGetTagFromTagID)(PDB, TAGID);
static BOOL (WINAPI *pSdbWriteNULLTag)(PDB, TAG);
static BOOL (WINAPI *pSdbWriteWORDTag)(PDB, TAG, WORD);
static BOOL (WINAPI *pSdbWriteDWORDTag)(PDB, TAG, DWORD);
static BOOL (WINAPI *pSdbWriteQWORDTag)(PDB, TAG, QWORD);
static BOOL (WINAPI *pSdbWriteBinaryTagFromFile)(PDB, TAG, LPCWSTR);
static BOOL (WINAPI *pSdbWriteStringTag)(PDB, TAG, LPCWSTR);
static BOOL (WINAPI *pSdbWriteStringRefTag)(PDB, TAG, TAGID);
static TAGID (WINAPI *pSdbBeginWriteListTag)(PDB, TAG);
static BOOL (WINAPI *pSdbEndWriteListTag)(PDB, TAGID);
static TAGID (WINAPI *pSdbFindFirstTag)(PDB, TAGID, TAG);
static TAGID (WINAPI *pSdbFindNextTag)(PDB, TAGID, TAGID);
static WORD (WINAPI *pSdbReadWORDTag)(PDB, TAGID, WORD);
static DWORD (WINAPI *pSdbReadDWORDTag)(PDB, TAGID, DWORD);
static QWORD (WINAPI *pSdbReadQWORDTag)(PDB, TAGID, QWORD);
static BOOL (WINAPI *pSdbReadBinaryTag)(PDB, TAGID, PBYTE, DWORD);
static BOOL (WINAPI *pSdbReadStringTag)(PDB, TAGID, LPWSTR, DWORD);
static DWORD (WINAPI *pSdbGetTagDataSize)(PDB, TAGID);
static PVOID (WINAPI *pSdbGetBinaryTagData)(PDB, TAGID);
static LPWSTR (WINAPI *pSdbGetStringTagPtr)(PDB, TAGID);
static TAGID (WINAPI *pSdbGetFirstChild)(PDB, TAGID);
static TAGID (WINAPI *pSdbGetNextChild)(PDB, TAGID, TAGID);
static BOOL (WINAPI *pSdbGetDatabaseID)(PDB, GUID*);
static BOOL (WINAPI *pSdbGUIDToString)(CONST GUID *, PCWSTR, SIZE_T);

DEFINE_GUID(GUID_DATABASE_TEST,0xe39b0eb0,0x55db,0x450b,0x9b,0xd4,0xd2,0x0c,0x94,0x84,0x26,0x0f);


static void Write(HANDLE file, LPCVOID buffer, DWORD size)
{
    DWORD dwWritten = 0;
    WriteFile(file, buffer, size, &dwWritten, NULL);
}

static void test_Sdb(void)
{
    static const WCHAR temp[] = {'t','e','m','p',0};
    static const WCHAR path1[] = {'t','e','m','p','.','s','d','b',0};
    static const WCHAR path2[] = {'t','e','m','p','2','.','b','i','n',0};
    static const WCHAR tag_size_string[] = {'S','I','Z','E',0};
    static const WCHAR tag_flag_lua_string[] = {'F','L','A','G','_','L','U','A',0};
    static const TAG tags[5] = {
        TAG_SIZE, TAG_FLAG_LUA, TAG_NAME,
        TAG_STRINGTABLE, TAG_STRINGTABLE_ITEM
    };
    WCHAR buffer[6] = {0};
    PDB pdb;
    QWORD qword;
    DWORD dword;
    WORD word;
    BOOL ret;
    HANDLE file; /* temp file created for testing purpose */
    TAG tag;
    TAGID tagid, ptagid, stringref = 6;
    LPCWSTR string;
    PBYTE binary;

    pdb = pSdbCreateDatabase(path1, DOS_PATH);
    ok (pdb != NULL, "failed to create database\n");
    if(pdb != NULL)
    {
        ret = pSdbWriteDWORDTag(pdb, tags[0], 0xDEADBEEF);
        ok (ret, "failed to write DWORD tag\n");
        ret = pSdbWriteQWORDTag(pdb, tags[1], 0xDEADBEEFBABE);
        ok (ret, "failed to write QWORD tag\n");
        ret = pSdbWriteStringRefTag(pdb, tags[2], stringref);
        ok (ret, "failed to write stringref tag\n");
        tagid = pSdbBeginWriteListTag(pdb, tags[3]);
        ok (tagid != TAGID_NULL, "unexpected NULL tagid\n");
        ret = pSdbWriteStringTag(pdb, tags[4], temp);
        ok (ret, "failed to write string tag\n");
        ret = pSdbWriteNULLTag(pdb, TAG_GENERAL);
        ok (ret, "failed to write NULL tag\n");
        ret = pSdbWriteWORDTag(pdb, TAG_MATCH_MODE, 0xACE);
        ok (ret, "failed to write WORD tag\n");
        ret = pSdbEndWriteListTag(pdb, tagid);
        ok (ret, "failed to update list size\n");
        /* [Err ][SdbCloseDatabase    ] Failed to close the file. */
        pSdbCloseDatabaseWrite(pdb);
    }

    /* [Err ][SdbGetDatabaseID    ] Failed to get root tag */
    pdb = pSdbOpenDatabase(path1, DOS_PATH);
    ok(pdb != NULL, "unexpected NULL handle\n");

    if(pdb)
    {
        tagid = pSdbGetFirstChild(pdb, TAGID_ROOT);
        ok(tagid == _TAGID_ROOT, "unexpected tagid %u, expected %u\n", tagid, _TAGID_ROOT);

        tag = pSdbGetTagFromTagID(pdb, tagid);
        ok(tag == TAG_SIZE, "unexpected tag 0x%x, expected 0x%x\n", tag, TAG_SIZE);

        string = pSdbTagToString(tag);
        ok(lstrcmpW(string, tag_size_string) == 0, "unexpected string %s, expected %s\n",
           wine_dbgstr_w(string), wine_dbgstr_w(tag_size_string));

        dword = pSdbReadDWORDTag(pdb, tagid, 0);
        ok(dword == 0xDEADBEEF, "unexpected value %u, expected 0xDEADBEEF\n", dword);

        tagid = pSdbGetNextChild(pdb, TAGID_ROOT, tagid);
        ok(tagid == _TAGID_ROOT + sizeof(TAG) + sizeof(DWORD), "unexpected tagid %u, expected %u\n",
           tagid, _TAGID_ROOT + sizeof(TAG) + sizeof(DWORD));

        tag = pSdbGetTagFromTagID(pdb, tagid);
        ok (tag == TAG_FLAG_LUA, "unexpected tag 0x%x, expected 0x%x\n", tag, TAG_FLAG_LUA);

        string = pSdbTagToString(tag);
        ok(lstrcmpW(string, tag_flag_lua_string) == 0, "unexpected string %s, expected %s\n",
           wine_dbgstr_w(string), wine_dbgstr_w(tag_flag_lua_string));

        qword = pSdbReadQWORDTag(pdb, tagid, 0);
        ok(qword == 0xDEADBEEFBABE, "unexpected value 0x%I64x, expected 0xDEADBEEFBABE\n", qword);

        tagid = pSdbGetNextChild(pdb, TAGID_ROOT, tagid);
        string = pSdbGetStringTagPtr(pdb, tagid);
        ok (string && (lstrcmpW(string, temp) == 0), "unexpected string %s, expected %s\n",
            wine_dbgstr_w(string), wine_dbgstr_w(temp));

        ptagid = pSdbGetNextChild(pdb, TAGID_ROOT, tagid);
        tagid = pSdbGetFirstChild(pdb, ptagid);

        string = pSdbGetStringTagPtr(pdb, tagid);
        ok (string && (lstrcmpW(string, temp) == 0), "unexpected string %s, expected %s\n",
            wine_dbgstr_w(string), wine_dbgstr_w(temp));

        ok (pSdbReadStringTag(pdb, tagid, buffer, 6), "failed to write string to buffer\n");
        /* [Err ][SdbpReadTagData     ] Buffer too small. Avail: 6, Need: 10. */
        ok (!pSdbReadStringTag(pdb, tagid, buffer, 3), "string was written to buffer, but failure was expected");
        ok (pSdbGetTagDataSize(pdb, tagid) == 5 * sizeof(WCHAR), "string has unexpected size\n");

        tagid = pSdbGetNextChild(pdb, ptagid, tagid);
        tag = pSdbGetTagFromTagID(pdb, tagid);
        ok (tag == TAG_GENERAL, "unexpected tag 0x%x, expected 0x%x\n", tag, TAG_GENERAL);
        ok (pSdbGetTagDataSize(pdb, tagid) == 0, "null tag with size > 0\n");

        tagid = pSdbGetNextChild(pdb, ptagid, tagid);
        word = pSdbReadWORDTag(pdb, tagid, 0);
        ok (word == 0xACE, "unexpected value 0x%x, expected 0x%x\n", word, 0xACE);

        pSdbCloseDatabase(pdb);
    }
    DeleteFileW(path1);

    file = CreateFileW(path2, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ok (file != INVALID_HANDLE_VALUE, "failed to open file\n");
    Write(file, &qword, 8);
    CloseHandle(file);

    pdb = pSdbCreateDatabase(path1, DOS_PATH);
    ok(pdb != NULL, "unexpected NULL handle\n");

    if(pdb)
    {
        ret = pSdbWriteBinaryTagFromFile(pdb, TAG_DATA_BITS, path2);
        ok(ret, "failed to write tag from binary file\n");
        pSdbCloseDatabaseWrite(pdb);     /* [Err ][SdbCloseDatabase    ] Failed to close the file. */
        DeleteFileW(path2);

        pdb = pSdbOpenDatabase(path1, DOS_PATH);
        ok(pdb != NULL, "unexpected NULL handle\n");
        binary = pSdbGetBinaryTagData(pdb, _TAGID_ROOT);
        ok(memcmp(binary, &qword, 8) == 0, "binary data is corrupt\n");
        ret = pSdbReadBinaryTag(pdb, _TAGID_ROOT, (PBYTE)buffer, 12);
        ok(ret, "failed to read binary tag\n");
        ok(memcmp(buffer, &qword, 8) == 0, "binary data is corrupt\n");
        pSdbCloseDatabase(pdb);
    }
    DeleteFileW(path1);
}

static void match_str_attr_imp(PDB pdb, TAGID parent, TAG find, const char* compare)
{
    TAGID attr = pSdbFindFirstTag(pdb, parent, find);
    winetest_ok(attr != TAG_NULL, "Could not find: %x\n", find);
    if (attr != TAG_NULL)
    {
        LPWSTR name = pSdbGetStringTagPtr(pdb, attr);
        winetest_ok(name != NULL, "Could not convert attr to str.\n");
        if (name)
        {
            char name_a[100];
            WideCharToMultiByte(CP_ACP, 0, name, -1, name_a, sizeof(name_a), NULL, NULL);
            winetest_ok(strcmp(name_a, compare) == 0, "Expected tagid %x to be %s, was %s\n", attr, compare, name_a);
        }
    }
}

static void match_dw_attr_imp(PDB pdb, TAGID parent, TAG find, DWORD compare)
{
    TAGID attr = pSdbFindFirstTag(pdb, parent, find);
    winetest_ok(attr != TAG_NULL, "Could not find: %x\n", find);
    if (attr != TAG_NULL)
    {
        DWORD val = pSdbReadDWORDTag(pdb, attr, 0x1234567);
        winetest_ok(val == compare, "Expected tagid %x to be 0x%x, was 0x%x\n", attr, compare, val);
    }
}

static void match_qw_attr_imp(PDB pdb, TAGID parent, TAG find, QWORD compare)
{
    TAGID attr = pSdbFindFirstTag(pdb, parent, find);
    winetest_ok(attr != TAG_NULL, "Could not find: %x\n", find);
    if (attr != TAG_NULL)
    {
        QWORD val = pSdbReadQWORDTag(pdb, attr, 0x123456789abcdef);
        winetest_ok(val == compare, "Expected tagid %x to be 0x%I64x, was 0x%I64x\n", attr, compare, val);
    }
}

static void match_guid_attr_imp(PDB pdb, TAGID parent, TAG find, const GUID* compare)
{
    TAGID attr = pSdbFindFirstTag(pdb, parent, find);
    winetest_ok(attr != TAG_NULL, "Could not find: %x\n", find);
    if (attr != TAG_NULL)
    {
        GUID guid = {0};
        BOOL result = pSdbReadBinaryTag(pdb, attr, (PBYTE)&guid, sizeof(guid));
        winetest_ok(result, "expected pSdbReadBinaryTag not to fail.\n");
        winetest_ok(IsEqualGUID(&guid, compare), "expected guids to be equal(%s:%s)\n", wine_dbgstr_guid(&guid), wine_dbgstr_guid(compare));
    }
}

#define match_str_attr  (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : match_str_attr_imp
#define match_dw_attr  (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : match_dw_attr_imp
#define match_qw_attr  (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : match_qw_attr_imp
#define match_guid_attr  (winetest_set_location(__FILE__, __LINE__), 0) ? (void)0 : match_guid_attr_imp


//The application name cannot contain any of the following characters:
// \ / < > : * ? |  "

static void check_db_properties(PDB pdb, TAGID root)
{
    TAGID iter = pSdbFindFirstTag(pdb, root, TAG_DATABASE_ID);
    ok(iter != TAGID_NULL, "expected a result, got TAGID_NULL\n");
    if(iter != TAGID_NULL)
    {
        GUID guid = {0}, guid2 = {0};
        BOOL result = pSdbReadBinaryTag(pdb, iter, (PBYTE)&guid, sizeof(guid));
        ok(result, "expected SdbReadBinaryTag not to fail.\n");
        if(result)
        {
            WCHAR guid_wstr[50];
            result = pSdbGUIDToString(&guid, guid_wstr, 50);
            ok(result, "expected SdbGUIDToString not to fail.\n");
            if(result)
            {
                char guid_str[50];
                WideCharToMultiByte( CP_ACP, 0, guid_wstr, -1, guid_str, sizeof(guid_str), NULL, NULL );
                ok_str(guid_str, "{e39b0eb0-55db-450b-9bd4-d20c9484260f}");
            }
            ok(pSdbGetDatabaseID(pdb, &guid2),"expected SdbGetDatabaseID not to fail.\n");
            ok(IsEqualGUID(&guid, &guid2), "expected guids to be equal(%s:%s)\n", wine_dbgstr_guid(&guid), wine_dbgstr_guid(&guid2));
        }
    }
    match_qw_attr(pdb, root, TAG_TIME, 0x1d1b91a02c0d63e);
    match_str_attr(pdb, root, TAG_COMPILER_VERSION, "2.1.0.3");
    match_str_attr(pdb, root, TAG_NAME, "apphelp_test1");
    match_dw_attr(pdb, root, TAG_OS_PLATFORM, 1);
}

static void check_db_layer(PDB pdb, TAGID layer)
{
    TAGID shimref, inexclude, is_include;
    ok(layer != TAGID_NULL, "Expected a valid layer, got NULL\n");
    if(!layer)
        return;

    match_str_attr(pdb, layer, TAG_NAME, "TestNewMode");
    shimref = pSdbFindFirstTag(pdb, layer, TAG_SHIM_REF);
    ok(shimref != TAGID_NULL, "Expected a valid shim ref, got NULL\n");
    if(!shimref)
        return;

    match_str_attr(pdb, shimref, TAG_NAME, "VirtualRegistry");
    match_str_attr(pdb, shimref, TAG_COMMAND_LINE, "ThemeActive");
    inexclude = pSdbFindFirstTag(pdb, shimref, TAG_INEXCLUD);
    ok(inexclude != TAGID_NULL, "Expected a valid in/exclude ref, got NULL\n");
    if(!inexclude)
        return;

    is_include = pSdbFindFirstTag(pdb, inexclude, TAG_INCLUDE);
    ok(is_include == TAGID_NULL, "Expected a NULL include ref, but got one anyway.\n");
    match_str_attr(pdb, inexclude, TAG_MODULE, "exclude.dll");

    inexclude = pSdbFindNextTag(pdb, shimref, inexclude);
    ok(inexclude != TAGID_NULL, "Expected a valid in/exclude ref, got NULL\n");
    if(!inexclude)
        return;

    is_include = pSdbFindFirstTag(pdb, inexclude, TAG_INCLUDE);
    ok(is_include != TAGID_NULL, "Expected a valid include ref, got NULL\n");
    match_str_attr(pdb, inexclude, TAG_MODULE, "include.dll");
}

static void check_matching_file(PDB pdb, TAGID exe, TAGID matching_file, int num)
{
    ok(matching_file != TAGID_NULL, "Expected to find atleast 1 matching file.\n");
    if(matching_file == TAGID_NULL)
        return;

    ok(num < 4, "Too many matches, expected only 4!\n");
    if (num >= 4)
        return;


    match_str_attr(pdb, matching_file, TAG_NAME, "*");
    match_str_attr(pdb, matching_file, TAG_COMPANY_NAME, "CompanyName");
    match_str_attr(pdb, matching_file, TAG_PRODUCT_NAME, "ProductName");
    match_str_attr(pdb, matching_file, TAG_PRODUCT_VERSION, "1.0.0.1");
    match_str_attr(pdb, matching_file, TAG_FILE_VERSION, "1.0.0.0");

    if( num == 0 || num == 3)
    {
        match_qw_attr(pdb, matching_file, TAG_UPTO_BIN_PRODUCT_VERSION, 0x1000000000001);
        match_qw_attr(pdb, matching_file, TAG_UPTO_BIN_FILE_VERSION, 0x1000000000000);
    }
    if(num == 1 || num == 3)
    {
        match_dw_attr(pdb, matching_file, TAG_PE_CHECKSUM, 0xbaad);
    }
    if(num != 0)
    {
        match_qw_attr(pdb, matching_file, TAG_BIN_PRODUCT_VERSION, 0x1000000000001);
        match_qw_attr(pdb, matching_file, TAG_BIN_FILE_VERSION, 0x1000000000000);
    }
    if(num == 3)
    {
        match_dw_attr(pdb, matching_file, TAG_SIZE, 0x800);
        match_dw_attr(pdb, matching_file, TAG_CHECKSUM, 0x178bd629);
        match_str_attr(pdb, matching_file, TAG_FILE_DESCRIPTION, "FileDescription");
        match_dw_attr(pdb, matching_file, TAG_MODULE_TYPE, 3);
        match_dw_attr(pdb, matching_file, TAG_VERFILEOS, 4);
        match_dw_attr(pdb, matching_file, TAG_VERFILETYPE, 1);
        match_dw_attr(pdb, matching_file, TAG_LINKER_VERSION, 0x40002);
        match_str_attr(pdb, matching_file, TAG_ORIGINAL_FILENAME, "OriginalFilename");
        match_str_attr(pdb, matching_file, TAG_INTERNAL_NAME, "InternalName");
        match_str_attr(pdb, matching_file, TAG_LEGAL_COPYRIGHT, "LegalCopyright");
        match_dw_attr(pdb, matching_file, TAG_LINK_DATE, 0x12345);
        match_dw_attr(pdb, matching_file, TAG_UPTO_LINK_DATE, 0x12345);
    }
    if(num > 3)
    {
        ok(0, "unknown case: %d\n", num);
    }
    matching_file = pSdbFindNextTag(pdb, exe, matching_file);
    if(num == 2)
    {
        ok(matching_file != TAGID_NULL, "Did expect a secondary match on %d\n", num);
        match_str_attr(pdb, matching_file, TAG_NAME, "test_checkfile.txt");
        match_dw_attr(pdb, matching_file, TAG_SIZE, 0x4);
        match_dw_attr(pdb, matching_file, TAG_CHECKSUM, 0xb0b0b0b0);
    }
    else
    {
        ok(matching_file == TAGID_NULL, "Did not expect a secondary match on %d\n", num);
    }
}

static void check_matching_apphelp(PDB pdb, TAGID apphelp, int num)
{
    if(num == 0)
    {
/*
[Window Title]
Program Compatibility Assistant

[Main Instruction]
This program has known compatibility issues

[Expanded Information]
Allow it!

[^] Hide details  [ ] Don't show this message again  [Check for solutions online] [Run program] [Cancel]
*/
        match_dw_attr(pdb, apphelp, TAG_FLAGS, 1);
        match_dw_attr(pdb, apphelp, TAG_PROBLEMSEVERITY, 1);
        match_dw_attr(pdb, apphelp, TAG_HTMLHELPID, 1);
        match_dw_attr(pdb, apphelp, TAG_APP_NAME_RC_ID, 0x6f0072);
        match_dw_attr(pdb, apphelp, TAG_VENDOR_NAME_RC_ID, 0x720067);
        match_dw_attr(pdb, apphelp, TAG_SUMMARY_MSG_RC_ID, 0);
    }
    else
    {
/*
[Window Title]
Program Compatibility Assistant

[Main Instruction]
This program is blocked due to compatibility issues

[Expanded Information]
Not allowed!

[^] Hide details  [Check for solutions online] [Cancel]
*/
        match_dw_attr(pdb, apphelp, TAG_FLAGS, 1);
        match_dw_attr(pdb, apphelp, TAG_PROBLEMSEVERITY, 2);
        match_dw_attr(pdb, apphelp, TAG_HTMLHELPID, 2);
        match_dw_attr(pdb, apphelp, TAG_APP_NAME_RC_ID, 0x320020);
        match_dw_attr(pdb, apphelp, TAG_VENDOR_NAME_RC_ID, 0x38002e);
        match_dw_attr(pdb, apphelp, TAG_SUMMARY_MSG_RC_ID, 0);
    }
    apphelp = pSdbFindNextTag(pdb, apphelp, apphelp);
    ok(apphelp == TAGID_NULL, "Did not expect a secondary match on %d\n", num);
}

static void check_matching_layer(PDB pdb, TAGID layer, int num)
{
    if(num == 2)
    {
        match_dw_attr(pdb, layer, TAG_LAYER_TAGID, 0x18e);
        match_str_attr(pdb, layer, TAG_NAME, "TestNewMode");
    }
    else
    {
        TAGID layer_tagid = pSdbFindFirstTag(pdb, layer, TAG_LAYER_TAGID);
        ok(layer_tagid == TAGID_NULL, "expected not to find a layer tagid, got %x\n", layer_tagid);
        match_str_attr(pdb, layer, TAG_NAME, "WinSrv03");
    }
}

static struct
{
    const char* name;
    const char* app_name;
    const char* vendor;
    GUID exe_id;
    const char* extra_file;
    DWORD dwLayerCount;
    TAGREF atrExes_0;
    TAGREF atrLayers_0;
} test_exedata[4] = {
    {
        "test_allow.exe",
        "apphelp_name_allow",
        "apphelp_vendor_allow",
        {0x4e50c93f,0xb863,0x4dfa,{0xba,0xe2,0xd8,0x0e,0xf4,0xce,0x5c,0x89}},
        NULL,
        0,
        0x1c6,
        0,
    },
    {
        "test_disallow.exe",
        "apphelp_name_disallow",
        "apphelp_vendor_disallow",
        {0x156720e1,0xef98,0x4d04,{0x96,0x5a,0xd8,0x5d,0xe0,0x5e,0x6d,0x9f}},
        NULL,
        0,
        0x256,
        0,
    },
    {
        "test_new.exe",
        "fixnew_name",
        "fixnew_vendor",
        {0xce70ef69,0xa21d,0x408b,{0x84,0x5b,0xf9,0x9e,0xac,0x06,0x09,0xe7}},
        "test_checkfile.txt",
        1,
        0x2ec,
        0x18e,
    },
    {
        "test_w2k3.exe",
        "fix_name",
        "fix_vendor",
        {0xb4ead144,0xf640,0x4e4b,{0x94,0xc4,0x0c,0x7f,0xa8,0x66,0x23,0xb0}},
        NULL,
        0,
        0x37c,
        0,
    },
};

static void check_db_exes(PDB pdb, TAGID root)
{
    int num = 0;
    TAGID exe = pSdbFindFirstTag(pdb, root, TAG_EXE);
    while (exe != TAGID_NULL)
    {
        TAGID apphelp, layer;
        ok(num < 4, "Too many matches, expected only 4!\n");
        if(num >= 4)
            break;
        match_str_attr(pdb, exe, TAG_NAME, test_exedata[num].name);
        match_str_attr(pdb, exe, TAG_APP_NAME, test_exedata[num].app_name);
        match_str_attr(pdb, exe, TAG_VENDOR, test_exedata[num].vendor);
        match_guid_attr(pdb, exe, TAG_EXE_ID, &test_exedata[num].exe_id);
        check_matching_file(pdb, exe, pSdbFindFirstTag(pdb, exe, TAG_MATCHING_FILE), num);
        apphelp = pSdbFindFirstTag(pdb, exe, TAG_APPHELP);
        if(num == 0 || num == 1)
        {
            ok(apphelp != TAGID_NULL, "Expected to find a valid apphelp match on %d.\n", num);
            if(apphelp)
                check_matching_apphelp(pdb, apphelp, num);
        }
        else
        {
            ok(apphelp == TAGID_NULL, "Did not expect an apphelp match on %d\n", num);
        }
        layer = pSdbFindFirstTag(pdb, exe, TAG_LAYER);
        if(num == 2 || num == 3)
        {
            ok(layer != TAGID_NULL, "Expected to find a valid layer match on %d.\n", num);
            if(layer)
                check_matching_layer(pdb, layer, num);
        }
        else
        {
            ok(layer == TAGID_NULL, "Did not expect a layer match on %d\n", num);
        }
        ++num;
        exe = pSdbFindNextTag(pdb, root, exe);
    }
    ok(num == 4, "Expected to find 4 exe tags, found: %d\n", num);
}

static struct
{
    DWORD htmlhelpid;
    const char* link;
    const char* apphelp_title;
    const char* apphelp_details;
} test_layerdata[2] = {
    {
        2,
        "http://reactos.org/disallow",
        "apphelp_name_disallow",
        "Not allowed!",
    },
    {
        1,
        "http://reactos.org/allow",
        "apphelp_name_allow",
        "Allow it!",
    },
};

static void check_db_apphelp(PDB pdb, TAGID root)
{
    int num = 0;
    TAGID apphelp = pSdbFindFirstTag(pdb, root, TAG_APPHELP);
    while (apphelp != TAGID_NULL)
    {
        TAGID link;
        ok(num < 2, "Too many matches, expected only 4!\n");
        if(num >= 2)
            break;
        match_dw_attr(pdb, apphelp, TAG_HTMLHELPID, test_layerdata[num].htmlhelpid);
        link = pSdbFindFirstTag(pdb, apphelp, TAG_LINK);
        ok(link != TAGID_NULL, "expected to find a link tag\n");
        if (link != TAGID_NULL)
        {
            match_str_attr(pdb, link, TAG_LINK_URL, test_layerdata[num].link);
        }
        match_str_attr(pdb, apphelp, TAG_APPHELP_TITLE, test_layerdata[num].apphelp_title);
        match_str_attr(pdb, apphelp, TAG_APPHELP_DETAILS, test_layerdata[num].apphelp_details);
        apphelp = pSdbFindNextTag(pdb, root, apphelp);
        num++;
    }
    ok(num == 2, "Expected to find 2 layer tags, found: %d\n", num);
}

static void test_CheckDatabaseManually(void)
{
    static const WCHAR path[] = {'t','e','s','t','_','d','b','.','s','d','b',0};
    TAGID root;
    PDB pdb;
    BOOL ret;
    DWORD ver_hi, ver_lo;

    test_create_db("test_db.sdb");

    /* both ver_hi and ver_lo cannot be null, it'll crash. */
    ver_hi = ver_lo = 0x12345678;
    ret = pSdbGetDatabaseVersion(path, &ver_hi, &ver_lo);
    ok(ret,"Expected SdbGetDatabaseVersion to succeed\n");
    ok(ver_hi == 2,"Expected ver_hi to be 2, was: %d\n", ver_hi);
    ok(ver_lo == 1,"Expected ver_lo to be 1, was: %d\n", ver_lo);

    ver_hi = ver_lo = 0x12345678;
    ret = pSdbGetDatabaseVersion(NULL, &ver_hi, &ver_lo);
    ok(ret,"Expected SdbGetDatabaseVersion to succeed\n");
    ok(ver_hi == 0x12345678,"Expected ver_hi to be 0x12345678, was: 0x%x\n", ver_hi);
    ok(ver_lo == 0x12345678,"Expected ver_lo to be 0x12345678, was: 0x%x\n", ver_lo);

    ver_hi = ver_lo = 0x12345678;
    ret = pSdbGetDatabaseVersion(path + 1, &ver_hi, &ver_lo);
    ok(ret,"Expected SdbGetDatabaseVersion to succeed\n");
    ok(ver_hi == 0x12345678,"Expected ver_hi to be 0x12345678, was: 0x%x\n", ver_hi);
    ok(ver_lo == 0x12345678,"Expected ver_lo to be 0x12345678, was: 0x%x\n", ver_lo);

    pdb = pSdbOpenDatabase(path, DOS_PATH);
    ok(pdb != NULL, "unexpected NULL handle\n");

    root = pSdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    ok(root != TAGID_NULL, "expected to find a root tag\n");
    if (root != TAGID_NULL)
    {
        check_db_properties(pdb, root);
        check_db_layer(pdb, pSdbFindFirstTag(pdb, root, TAG_LAYER));
        check_db_exes(pdb, root);
        check_db_apphelp(pdb, root);
    }

    pSdbCloseDatabase(pdb);
    DeleteFileA("test_db.sdb");
}

START_TEST(db)
{
    //SetEnvironmentVariable("SHIM_DEBUG_LEVEL", "4");
    //SetEnvironmentVariable("DEBUGCHANNEL", "+apphelp");
    hdll = LoadLibraryA("apphelp.dll");
    pSdbTagToString = (void *) GetProcAddress(hdll, "SdbTagToString");
    pSdbOpenDatabase = (void *) GetProcAddress(hdll, "SdbOpenDatabase");
    pSdbCreateDatabase = (void *) GetProcAddress(hdll, "SdbCreateDatabase");
    pSdbGetDatabaseVersion = (void *) GetProcAddress(hdll, "SdbGetDatabaseVersion");
    pSdbCloseDatabase = (void *) GetProcAddress(hdll, "SdbCloseDatabase");
    pSdbCloseDatabaseWrite = (void *) GetProcAddress(hdll, "SdbCloseDatabaseWrite");
    pSdbGetTagFromTagID = (void *) GetProcAddress(hdll, "SdbGetTagFromTagID");
    pSdbWriteNULLTag = (void *) GetProcAddress(hdll, "SdbWriteNULLTag");
    pSdbWriteWORDTag = (void *) GetProcAddress(hdll, "SdbWriteWORDTag");
    pSdbWriteDWORDTag = (void *) GetProcAddress(hdll, "SdbWriteDWORDTag");
    pSdbWriteQWORDTag = (void *) GetProcAddress(hdll, "SdbWriteQWORDTag");
    pSdbWriteBinaryTagFromFile = (void *) GetProcAddress(hdll, "SdbWriteBinaryTagFromFile");
    pSdbWriteStringTag = (void *) GetProcAddress(hdll, "SdbWriteStringTag");
    pSdbWriteStringRefTag = (void *) GetProcAddress(hdll, "SdbWriteStringRefTag");
    pSdbBeginWriteListTag = (void *)GetProcAddress(hdll, "SdbBeginWriteListTag");
    pSdbEndWriteListTag = (void *) GetProcAddress(hdll, "SdbEndWriteListTag");
    pSdbFindFirstTag = (void *) GetProcAddress(hdll, "SdbFindFirstTag");
    pSdbFindNextTag = (void *) GetProcAddress(hdll, "SdbFindNextTag");
    pSdbReadWORDTag = (void *) GetProcAddress(hdll, "SdbReadWORDTag");
    pSdbReadDWORDTag = (void *) GetProcAddress(hdll, "SdbReadDWORDTag");
    pSdbReadQWORDTag = (void *) GetProcAddress(hdll, "SdbReadQWORDTag");
    pSdbReadBinaryTag = (void *) GetProcAddress(hdll, "SdbReadBinaryTag");
    pSdbReadStringTag = (void *) GetProcAddress(hdll, "SdbReadStringTag");
    pSdbGetTagDataSize = (void *) GetProcAddress(hdll, "SdbGetTagDataSize");
    pSdbGetBinaryTagData = (void *) GetProcAddress(hdll, "SdbGetBinaryTagData");
    pSdbGetStringTagPtr = (void *) GetProcAddress(hdll, "SdbGetStringTagPtr");
    pSdbGetFirstChild = (void *) GetProcAddress(hdll, "SdbGetFirstChild");
    pSdbGetNextChild = (void *) GetProcAddress(hdll, "SdbGetNextChild");
    pSdbGetDatabaseID = (void *) GetProcAddress(hdll, "SdbGetDatabaseID");
    pSdbGUIDToString = (void *) GetProcAddress(hdll, "SdbGUIDToString");

    test_Sdb();
    test_CheckDatabaseManually();
}