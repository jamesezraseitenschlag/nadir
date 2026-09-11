// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifdef _WIN32
#define TokenType WIN32_TokenType
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef TokenType
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "metadata.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include <ctype.h>

static ProjectSchema g_project_schema = {NULL, 0, 0, NULL};

ProjectSchema* project_schema_get_instance(void) {
    return &g_project_schema;
}

void project_schema_reset(void) {
    ProjectSchema* ps = &g_project_schema;
    for (int i = 0; i < ps->object_count; i++) {
        MetaObject* obj = &ps->objects[i];
        if (obj->full_name) free(obj->full_name);
        if (obj->label) free(obj->label);
        if (obj->sharing_model) free(obj->sharing_model);
        for (int f = 0; f < obj->field_count; f++) {
            MetaField* mf = &obj->fields[f];
            if (mf->full_name) free(mf->full_name);
            if (mf->label) free(mf->label);
            if (mf->type) free(mf->type);
            if (mf->reference_to) free(mf->reference_to);
            if (mf->default_value) free(mf->default_value);
        }
        if (obj->fields) free(obj->fields);
    }
    if (ps->objects) free(ps->objects);
    if (ps->project_root) free(ps->project_root);
    ps->objects = NULL;
    ps->object_count = 0;
    ps->object_capacity = 0;
    ps->project_root = NULL;
}

MetaObject* project_schema_get_or_create_object(const char* obj_name) {
    if (!obj_name) return NULL;
    ProjectSchema* ps = &g_project_schema;
    for (int i = 0; i < ps->object_count; i++) {
        if (nadr_str_eq(ps->objects[i].full_name, obj_name)) {
            return &ps->objects[i];
        }
    }
    if (ps->object_count + 1 > ps->object_capacity) {
        ps->object_capacity = ps->object_capacity < 8 ? 8 : ps->object_capacity * 2;
        ps->objects = (MetaObject*)realloc(ps->objects, sizeof(MetaObject) * ps->object_capacity);
    }
    MetaObject* obj = &ps->objects[ps->object_count++];
    obj->full_name = nadr_strdup(obj_name);
    obj->label = nadr_strdup(obj_name);
    obj->sharing_model = nadr_strdup("ReadWrite");
    obj->fields = NULL;
    obj->field_count = 0;
    obj->field_capacity = 0;

    MockDB* db = mock_db_get_instance();
    bool exists = false;
    for (int t = 0; t < db->table_count; t++) {
        if (nadr_str_eq(db->tables[t].object_name, obj_name)) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        if (db->table_count + 1 > db->table_capacity) {
            db->table_capacity = db->table_capacity < 8 ? 8 : db->table_capacity * 2;
            db->tables = (DBTable*)realloc(db->tables, sizeof(DBTable) * db->table_capacity);
        }
        DBTable* t = &db->tables[db->table_count++];
        t->object_name = nadr_strdup(obj_name);
        t->records = NULL;
        t->record_count = 0;
        t->record_capacity = 0;
        t->auto_id_seq = 0;
    }
    return obj;
}

MetaObject* project_schema_get_object(const char* obj_name) {
    if (!obj_name) return NULL;
    ProjectSchema* ps = &g_project_schema;
    for (int i = 0; i < ps->object_count; i++) {
        if (nadr_str_eq(ps->objects[i].full_name, obj_name)) {
            return &ps->objects[i];
        }
    }
    return NULL;
}

void project_schema_add_field(MetaObject* obj, MetaField field) {
    if (!obj || !field.full_name) return;
    for (int i = 0; i < obj->field_count; i++) {
        if (nadr_str_eq(obj->fields[i].full_name, field.full_name)) {
            if (obj->fields[i].label) free(obj->fields[i].label);
            if (obj->fields[i].type) free(obj->fields[i].type);
            if (obj->fields[i].reference_to) free(obj->fields[i].reference_to);
            if (obj->fields[i].default_value) free(obj->fields[i].default_value);
            obj->fields[i] = field;
            return;
        }
    }
    if (obj->field_count + 1 > obj->field_capacity) {
        obj->field_capacity = obj->field_capacity < 8 ? 8 : obj->field_capacity * 2;
        obj->fields = (MetaField*)realloc(obj->fields, sizeof(MetaField) * obj->field_capacity);
    }
    obj->fields[obj->field_count++] = field;
}

MetaField* project_schema_get_field(const char* obj_name, const char* field_name) {
    MetaObject* obj = project_schema_get_object(obj_name);
    if (!obj || !field_name) return NULL;
    for (int i = 0; i < obj->field_count; i++) {
        if (nadr_str_eq(obj->fields[i].full_name, field_name)) {
            return &obj->fields[i];
        }
    }
    return NULL;
}

static char* xml_get_tag(const char* xml, const char* tag) {
    if (!xml || !tag) return NULL;
    int tlen = (int)strlen(tag);
    const char* cur = xml;
    const char* start = NULL;
    while ((cur = strstr(cur, "<")) != NULL) {
        if (strncmp(cur + 1, tag, tlen) == 0) {
            char next = cur[1 + tlen];
            if (next == '>' || isspace((unsigned char)next) || next == '/') {
                start = cur;
                break;
            }
        }
        cur++;
    }
    if (!start) return NULL;
    const char* content_start = strchr(start, '>');
    if (!content_start) return NULL;
    content_start++;

    char close_tag[128];
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag);
    const char* end = strstr(content_start, close_tag);
    if (!end) return NULL;

    while (content_start < end && isspace((unsigned char)*content_start)) content_start++;
    const char* content_end = end;
    while (content_end > content_start && isspace((unsigned char)*(content_end - 1))) content_end--;

    int len = (int)(content_end - content_start);
    if (len < 0) len = 0;
    return nadr_strndup(content_start, len);
}

static void parse_field_meta_file(const char* file_path, const char* obj_name, const char* filename) {
    char* xml = nadr_read_file(file_path);
    if (!xml) return;

    MetaObject* obj = project_schema_get_or_create_object(obj_name);
    MetaField mf = {0};

    char* fn = xml_get_tag(xml, "fullName");
    if (!fn) {
        char base[256];
        snprintf(base, sizeof(base), "%s", filename);
        char* dot = strstr(base, ".field-meta.xml");
        if (dot) *dot = '\0';
        fn = nadr_strdup(base);
    }
    mf.full_name = fn;
    mf.label = xml_get_tag(xml, "label");
    mf.type = xml_get_tag(xml, "type");
    mf.reference_to = xml_get_tag(xml, "referenceTo");
    mf.default_value = xml_get_tag(xml, "defaultValue");

    char* req = xml_get_tag(xml, "required");
    mf.required = req && (nadr_str_eq(req, "true") || nadr_str_eq(req, "1"));
    if (req) free(req);

    char* unq = xml_get_tag(xml, "unique");
    mf.unique = unq && (nadr_str_eq(unq, "true") || nadr_str_eq(unq, "1"));
    if (unq) free(unq);

    char* ext = xml_get_tag(xml, "externalId");
    mf.external_id = ext && (nadr_str_eq(ext, "true") || nadr_str_eq(ext, "1"));
    if (ext) free(ext);

    char* len_s = xml_get_tag(xml, "length");
    if (len_s) {
        mf.length = atoi(len_s);
        free(len_s);
    }
    project_schema_add_field(obj, mf);
    free(xml);
}

static void parse_object_meta_file(const char* file_path, const char* obj_name) {
    char* xml = nadr_read_file(file_path);
    if (!xml) return;

    MetaObject* obj = project_schema_get_or_create_object(obj_name);
    char* lbl = xml_get_tag(xml, "label");
    if (lbl) {
        if (obj->label) free(obj->label);
        obj->label = lbl;
    }
    char* sm = xml_get_tag(xml, "sharingModel");
    if (sm) {
        if (obj->sharing_model) free(obj->sharing_model);
        obj->sharing_model = sm;
    }
    free(xml);
}

static void parse_custom_metadata_file(const char* file_path, const char* filename) {
    char* xml = nadr_read_file(file_path);
    if (!xml) return;

    char type_name[256] = {0};
    char record_name[256] = {0};
    const char* p1 = filename;
    const char* dot1 = strchr(p1, '.');
    if (dot1) {
        int tlen = (int)(dot1 - p1);
        if (tlen > 250) tlen = 250;
        strncpy(type_name, p1, tlen);
        type_name[tlen] = '\0';

        const char* p2 = dot1 + 1;
        const char* dot2 = strstr(p2, ".md-meta.xml");
        if (dot2) {
            int rlen = (int)(dot2 - p2);
            if (rlen > 250) rlen = 250;
            strncpy(record_name, p2, rlen);
            record_name[rlen] = '\0';
        } else {
            strncpy(record_name, p2, 250);
        }
    } else {
        strncpy(type_name, filename, 250);
        strncpy(record_name, filename, 250);
    }

    char full_obj_type[300];
    if (strstr(type_name, "__mdt")) {
        snprintf(full_obj_type, sizeof(full_obj_type), "%s", type_name);
    } else {
        snprintf(full_obj_type, sizeof(full_obj_type), "%s__mdt", type_name);
    }

    project_schema_get_or_create_object(full_obj_type);
    SObject* sobj = sobject_new(full_obj_type);

    char* lbl = xml_get_tag(xml, "label");
    sobject_put(sobj, "DeveloperName", val_string(record_name));
    sobject_put(sobj, "MasterLabel", val_string(lbl ? lbl : record_name));
    sobject_put(sobj, "Label", val_string(lbl ? lbl : record_name));
    if (lbl) free(lbl);

    const char* cur = xml;
    while ((cur = strstr(cur, "<values>")) != NULL) {
        const char* v_end = strstr(cur, "</values>");
        if (!v_end) break;

        int block_len = (int)(v_end - cur) + 9;
        char* block = nadr_strndup(cur, block_len);
        if (block) {
            char* fld = xml_get_tag(block, "field");
            char* val_s = xml_get_tag(block, "value");
            if (fld && val_s) {
                sobject_put(sobj, fld, val_string(val_s));
            }
            if (fld) free(fld);
            if (val_s) free(val_s);
            free(block);
        }
        cur = v_end + 9;
    }

    mock_db_insert(sobj);
    free(xml);
}

static void scan_directory_meta(const char* dir_path, Interpreter* interp, int* objects_cnt, int* fields_cnt, int* cmdt_cnt, int* classes_cnt) {
#ifdef _WIN32
    char search_pattern[1024];
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*.*", dir_path);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;

        char full_item_path[1024];
        snprintf(full_item_path, sizeof(full_item_path), "%s\\%s", dir_path, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char norm_path[1024];
            strncpy(norm_path, full_item_path, sizeof(norm_path));
            for (char* p = norm_path; *p; p++) if (*p == '/') *p = '\\';

            if (strstr(norm_path, "\\objects\\") != NULL && strstr(norm_path, "\\fields") == NULL) {
                const char* obj_name = fd.cFileName;
                project_schema_get_or_create_object(obj_name);
                (*objects_cnt)++;

                char obj_meta_path[1024];
                snprintf(obj_meta_path, sizeof(obj_meta_path), "%s\\%s.object-meta.xml", full_item_path, obj_name);
                parse_object_meta_file(obj_meta_path, obj_name);
            }
            scan_directory_meta(full_item_path, interp, objects_cnt, fields_cnt, cmdt_cnt, classes_cnt);
        } else {
            const char* fn = fd.cFileName;
            if (strstr(fn, ".field-meta.xml")) {
                char norm[1024];
                strncpy(norm, full_item_path, sizeof(norm));
                for (char* p = norm; *p; p++) if (*p == '/') *p = '\\';
                char* obj_marker = strstr(norm, "\\objects\\");
                if (obj_marker) {
                    obj_marker += 9;
                    char* next_slash = strchr(obj_marker, '\\');
                    if (next_slash) {
                        int olen = (int)(next_slash - obj_marker);
                        char obj_buf[256];
                        if (olen > 250) olen = 250;
                        strncpy(obj_buf, obj_marker, olen);
                        obj_buf[olen] = '\0';
                        parse_field_meta_file(full_item_path, obj_buf, fn);
                        (*fields_cnt)++;
                    }
                }
            } else if (strstr(fn, ".md-meta.xml")) {
                parse_custom_metadata_file(full_item_path, fn);
                (*cmdt_cnt)++;
            } else if (strstr(fn, ".cls") && !strstr(fn, ".cls-meta.xml")) {
                if (interp) {
                    char* src = nadr_read_file(full_item_path);
                    if (src) {
                        Lexer l;
                        lexer_init(&l, src);
                        Parser p;
                        parser_init(&p, &l);
                        p.silent_mode = true;
                        ASTNode* prog = parser_parse(&p);
                        if (!p.had_error && prog && prog->type == NODE_PROGRAM) {
                            for (int i = 0; i < prog->as.program.statements.count; i++) {
                                if (prog->as.program.statements.nodes[i]->type == NODE_CLASS_DECL) {
                                    interpreter_register_class(interp, prog->as.program.statements.nodes[i]);
                                }
                            }
                        } else if (prog) {
                            ast_node_free(prog);
                        }
                        free(src);
                    }
                }
                (*classes_cnt)++;
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
#else
    DIR* dir = opendir(dir_path);
    if (!dir) return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char full_item_path[1024];
        snprintf(full_item_path, sizeof(full_item_path), "%s/%s", dir_path, entry->d_name);
        struct stat st;
        if (stat(full_item_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (strstr(full_item_path, "/objects/") != NULL && strstr(full_item_path, "/fields") == NULL) {
                    const char* obj_name = entry->d_name;
                    project_schema_get_or_create_object(obj_name);
                    (*objects_cnt)++;
                    char obj_meta_path[1024];
                    snprintf(obj_meta_path, sizeof(obj_meta_path), "%s/%s.object-meta.xml", full_item_path, obj_name);
                    parse_object_meta_file(obj_meta_path, obj_name);
                }
                scan_directory_meta(full_item_path, interp, objects_cnt, fields_cnt, cmdt_cnt, classes_cnt);
            } else {
                const char* fn = entry->d_name;
                if (strstr(fn, ".field-meta.xml")) {
                    char* obj_marker = strstr(full_item_path, "/objects/");
                    if (obj_marker) {
                        obj_marker += 9;
                        char* next_slash = strchr(obj_marker, '/');
                        if (next_slash) {
                            int olen = (int)(next_slash - obj_marker);
                            char obj_buf[256];
                            if (olen > 250) olen = 250;
                            strncpy(obj_buf, obj_marker, olen);
                            obj_buf[olen] = '\0';
                            parse_field_meta_file(full_item_path, obj_buf, fn);
                            (*fields_cnt)++;
                        }
                    }
                } else if (strstr(fn, ".md-meta.xml")) {
                    parse_custom_metadata_file(full_item_path, fn);
                    (*cmdt_cnt)++;
                } else if (strstr(fn, ".cls") && !strstr(fn, ".cls-meta.xml")) {
                    if (interp) {
                        char* src = nadr_read_file(full_item_path);
                        if (src) {
                            Lexer l;
                            lexer_init(&l, src);
                            Parser p;
                            parser_init(&p, &l);
                            p.silent_mode = true;
                            ASTNode* prog = parser_parse(&p);
                            if (!p.had_error && prog && prog->type == NODE_PROGRAM) {
                                for (int i = 0; i < prog->as.program.statements.count; i++) {
                                    if (prog->as.program.statements.nodes[i]->type == NODE_CLASS_DECL) {
                                        interpreter_register_class(interp, prog->as.program.statements.nodes[i]);
                                    }
                                }
                            } else if (prog) {
                                ast_node_free(prog);
                            }
                            free(src);
                        }
                    }
                    (*classes_cnt)++;
                }
            }
        }
    }
    closedir(dir);
#endif
}

int nadir_project_init(const char* root_path, Interpreter* interp) {
    if (!root_path) root_path = ".";
    ProjectSchema* ps = project_schema_get_instance();
    if (ps->project_root) free(ps->project_root);
    ps->project_root = nadr_strdup(root_path);

    int objects_cnt = 0;
    int fields_cnt = 0;
    int cmdt_cnt = 0;
    int classes_cnt = 0;

    printf("[nadir-init] Initializing project structure from \"%s\"...\n", root_path);
    scan_directory_meta(root_path, interp, &objects_cnt, &fields_cnt, &cmdt_cnt, &classes_cnt);

    printf("[nadir-init] Discovered %d objects, %d custom fields.\n", ps->object_count, fields_cnt);
    printf("[nadir-init] Loaded %d custom metadata records into database.\n", cmdt_cnt);
    printf("[nadir-init] Discovered & registered %d Apex classes.\n", classes_cnt);
    printf("[nadir-init] Project metadata and database structure initialized.\n\n");
    return 0;
}
