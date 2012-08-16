/*
 * Copyright (C) 2012
 *     Dale Weiler, Wolfgang Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "gmqcc.h"

/*
 * The macros below expand to a typesafe vector implementation, which
 * can be viewed in gmqcc.h
 *
 * code_statements_data      -- raw prog_section_statement array
 * code_statements_elements  -- number of elements
 * code_statements_allocated -- size of the array allocated
 * code_statements_add(T)    -- add element (returns -1 on error)
 *
 * code_vars_data            -- raw prog_section_var array
 * code_vars_elements        -- number of elements
 * code_vars_allocated       -- size of the array allocated
 * code_vars_add(T)          -- add element (returns -1 on error)
 *
 * code_fields_data          -- raw prog_section_field array
 * code_fields_elements      -- number of elements
 * code_fields_allocated     -- size of the array allocated
 * code_fields_add(T)        -- add element (returns -1 on error)
 *
 * code_functions_data       -- raw prog_section_function array
 * code_functions_elements   -- number of elements
 * code_functions_allocated  -- size of the array allocated
 * code_functions_add(T)     -- add element (returns -1 on error)
 *
 * code_globals_data         -- raw prog_section_def array
 * code_globals_elements     -- number of elements
 * code_globals_allocated    -- size of the array allocated
 * code_globals_add(T)       -- add element (returns -1 on error)
 *
 * code_chars_data           -- raw char* array
 * code_chars_elements       -- number of elements
 * code_chars_allocated      -- size of the array allocated
 * code_chars_add(T)         -- add element (returns -1 on error)
 */
VECTOR_MAKE(prog_section_statement, code_statements);
VECTOR_MAKE(prog_section_def,       code_defs      );
VECTOR_MAKE(prog_section_field,     code_fields    );
VECTOR_MAKE(prog_section_function,  code_functions );
VECTOR_MAKE(int,                    code_globals   );
VECTOR_MAKE(char,                   code_chars     );

uint32_t                            code_entfields;

void code_init() {
    prog_section_function  empty_function  = {0,0,0,0,0,0,0,{0}};
    prog_section_statement empty_statement = {0,{0},{0},{0}};
    int                    i               = 0;

    /* omit creation of null code */
    if (OPTS_FLAG(OMIT_NULL_BYTES))
        return;

    /*
     * The way progs.dat is suppose to work is odd, there needs to be
     * some null (empty) statements, functions, and 28 globals
     */
    for(; i < 28; i++)
        code_globals_add(0);

    code_chars_add     ('\0');
    code_functions_add (empty_function);
    code_statements_add(empty_statement);

    code_entfields = 0;
}

uint32_t code_genstring(const char *str)
{
    uint32_t off = code_chars_elements;
    while (*str) {
        code_chars_add(*str);
        ++str;
    }
    code_chars_add(0);
    return off;
}

uint32_t code_cachedstring(const char *str)
{
    size_t s = 0;
    /* We could implement knuth-morris-pratt or something
     * and also take substrings, but I'm uncomfortable with
     * pointing to subparts of strings for the sake of clarity...
     */
    while (s < code_chars_elements) {
        if (!strcmp(str, code_chars_data + s))
            return s;
        while (code_chars_data[s]) ++s;
        ++s;
    }
    return code_genstring(str);
}

void code_test() {
    prog_section_def       d1 = { TYPE_VOID,     28, 1 };
    prog_section_def       d2 = { TYPE_FUNCTION, 29, 8 };
    prog_section_def       d3 = { TYPE_STRING,   30, 14};
    prog_section_function  f1 = { 1, 0, 0, 0, 1,            0,0, {0}};
    prog_section_function  f2 = {-4, 0, 0, 0, 8,            0,0, {0}};
    prog_section_function  f3 = { 0, 0, 0, 0, 14+13,        0,0, {0}};
    prog_section_function  f4 = { 0, 0, 0, 0, 14+13+10,     0,0, {0}};
    prog_section_function  f5 = { 0, 0, 0, 0, 14+13+10+7,   0,0, {0}};
    prog_section_function  f6 = { 0, 0, 0, 0, 14+13+10+7+9, 0,0, {0}};
    prog_section_statement s1 = { INSTR_STORE_F, {30}, {OFS_PARM0}, {0}};
    prog_section_statement s2 = { INSTR_CALL1,   {29}, {0},         {0}};
    prog_section_statement s3 = { INSTR_RETURN,  {0},  {0},         {0}};

    code_chars_put("m_init",        0x6);
    code_chars_put("print",         0x5);
    code_chars_put("hello world\n", 0xC);
    code_chars_put("m_keydown",     0x9);
    code_chars_put("m_draw",        0x6);
    code_chars_put("m_toggle",      0x8);
    code_chars_put("m_shutdown",    0xA);

    code_globals_add(1);  /* m_init */
    code_globals_add(2);  /* print  */
    code_globals_add(14); /* hello world in string table */

    /* now the defs */
    code_defs_add      (d1); /* m_init    */
    code_defs_add      (d2); /* print     */
    code_defs_add      (d3); /*hello_world*/
    code_functions_add (f1); /* m_init    */
    code_functions_add (f2); /* print     */
    code_functions_add (f3); /* m_keydown */
    code_functions_add (f4);
    code_functions_add (f5);
    code_functions_add (f6);
    code_statements_add(s1);
    code_statements_add(s2);
    code_statements_add(s3);
}

qcint code_alloc_field (size_t qcsize)
{
    qcint pos = (qcint)code_entfields;
    code_entfields += qcsize;
    return pos;
}

bool code_write(const char *filename) {
    prog_header  code_header;
    FILE        *fp           = NULL;
    size_t       it           = 2;

    /* see proposal.txt */
    if (OPTS_FLAG(OMIT_NULL_BYTES)) {}
    code_header.statements.offset = sizeof(prog_header);
    code_header.statements.length = code_statements_elements;
    code_header.defs.offset       = code_header.statements.offset + (sizeof(prog_section_statement) * code_statements_elements);
    code_header.defs.length       = code_defs_elements;
    code_header.fields.offset     = code_header.defs.offset       + (sizeof(prog_section_def)       * code_defs_elements);
    code_header.fields.length     = code_fields_elements;
    code_header.functions.offset  = code_header.fields.offset     + (sizeof(prog_section_field)     * code_fields_elements);
    code_header.functions.length  = code_functions_elements;
    code_header.globals.offset    = code_header.functions.offset  + (sizeof(prog_section_function)  * code_functions_elements);
    code_header.globals.length    = code_globals_elements;
    code_header.strings.offset    = code_header.globals.offset    + (sizeof(int32_t)                * code_globals_elements);
    code_header.strings.length    = code_chars_elements;
    code_header.version           = 6;
    code_header.crc16             = 0; /* TODO: */
    code_header.entfield          = code_entfields;

    if (OPTS_FLAG(DARKPLACES_STRING_TABLE_BUG)) {
        util_debug("GEN", "Patching stringtable for -fdarkplaces-stringtablebug\n");

        /* >= + P */
        code_chars_add('\0'); /* > */
        code_chars_add('\0'); /* = */
        code_chars_add('\0'); /* P */
    }

    /* ensure all data is in LE format */
    util_endianswap(&code_header,          1,                       sizeof(prog_header));
    util_endianswap(code_statements_data, code_statements_elements, sizeof(prog_section_statement));
    util_endianswap(code_defs_data,       code_defs_elements,       sizeof(prog_section_def));
    util_endianswap(code_fields_data,     code_fields_elements,     sizeof(prog_section_field));
    util_endianswap(code_functions_data,  code_functions_elements,  sizeof(prog_section_function));
    util_endianswap(code_globals_data,    code_globals_elements,    sizeof(int32_t));

    fp = util_fopen(filename, "wb");
    if (!fp)
        return false;

    if (1 != fwrite(&code_header,         sizeof(prog_header), 1, fp) ||
        code_statements_elements != fwrite(code_statements_data, sizeof(prog_section_statement), code_statements_elements, fp) ||
        code_defs_elements       != fwrite(code_defs_data,       sizeof(prog_section_def)      , code_defs_elements      , fp) ||
        code_fields_elements     != fwrite(code_fields_data,     sizeof(prog_section_field)    , code_fields_elements    , fp) ||
        code_functions_elements  != fwrite(code_functions_data,  sizeof(prog_section_function) , code_functions_elements , fp) ||
        code_globals_elements    != fwrite(code_globals_data,    sizeof(int32_t)               , code_globals_elements   , fp) ||
        code_chars_elements      != fwrite(code_chars_data,      1                             , code_chars_elements     , fp))
    {
        fclose(fp);
        return false;
    }

    util_debug("GEN","HEADER:\n");
    util_debug("GEN","    version:    = %d\n", code_header.version );
    util_debug("GEN","    crc16:      = %d\n", code_header.crc16   );
    util_debug("GEN","    entfield:   = %d\n", code_header.entfield);
    util_debug("GEN","    statements  = {.offset = % 8d, .length = % 8d}\n", code_header.statements.offset, code_header.statements.length);
    util_debug("GEN","    defs        = {.offset = % 8d, .length = % 8d}\n", code_header.defs      .offset, code_header.defs      .length);
    util_debug("GEN","    fields      = {.offset = % 8d, .length = % 8d}\n", code_header.fields    .offset, code_header.fields    .length);
    util_debug("GEN","    functions   = {.offset = % 8d, .length = % 8d}\n", code_header.functions .offset, code_header.functions .length);
    util_debug("GEN","    globals     = {.offset = % 8d, .length = % 8d}\n", code_header.globals   .offset, code_header.globals   .length);
    util_debug("GEN","    strings     = {.offset = % 8d, .length = % 8d}\n", code_header.strings   .offset, code_header.strings   .length);

    /* FUNCTIONS */
    util_debug("GEN", "FUNCTIONS:\n");
    for (; it < code_functions_elements; it++) {
        size_t j = code_functions_data[it].entry;
        util_debug("GEN", "    {.entry =% 5d, .firstlocal =% 5d, .locals =% 5d, .profile =% 5d, .name =% 5d, .file =% 5d, .nargs =% 5d, .argsize ={%d,%d,%d,%d,%d,%d,%d,%d} }\n",
            code_functions_data[it].entry,
            code_functions_data[it].firstlocal,
            code_functions_data[it].locals,
            code_functions_data[it].profile,
            code_functions_data[it].name,
            code_functions_data[it].file,
            code_functions_data[it].nargs,
            code_functions_data[it].argsize[0],
            code_functions_data[it].argsize[1],
            code_functions_data[it].argsize[2],
            code_functions_data[it].argsize[3],
            code_functions_data[it].argsize[4],
            code_functions_data[it].argsize[5],
            code_functions_data[it].argsize[6],
            code_functions_data[it].argsize[7]

        );
        util_debug("GEN", "    NAME: %s\n", &code_chars_data[code_functions_data[it].name]);
        /* Internal functions have no code */
        if (code_functions_data[it].entry >= 0) {
            util_debug("GEN", "    CODE:\n");
            for (;;) {
                if (code_statements_data[j].opcode != AINSTR_END)
                    util_debug("GEN", "        %s {0x%05x,0x%05x,0x%05x}\n",
                        asm_instr[code_statements_data[j].opcode].m,
                        code_statements_data[j].o1.s1,
                        code_statements_data[j].o2.s1,
                        code_statements_data[j].o3.s1
                    );
                else {
                    util_debug("GEN", "        DONE  {0x00000,0x00000,0x00000}\n");
                    break;
                }
                j++;
            }
        }
    }

    mem_d(code_statements_data);
    mem_d(code_defs_data);
    mem_d(code_fields_data);
    mem_d(code_functions_data);
    mem_d(code_globals_data);
    mem_d(code_chars_data);
    fclose(fp);
    return true;
}
