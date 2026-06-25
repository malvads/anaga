#include "drivers/command/command.h"
#include "display/display.h"
#include "include/string.h"
#include "version/version.h"
#include "system/system.h"
#include "fs/vfs.h"
#include "fs/elf.h"

typedef void (*cmd_fn)(char*);
typedef struct { const char* name; cmd_fn fn; const char* desc; } cmd_t;

static void cmd_help(char* a);
static void cmd_ls(char* a)       { (void)a; vfs_ls(); }
static void cmd_pwd(char* a)      { (void)a; vfs_pwd(); kputc('\n'); }
static void cmd_cd(char* a)       { vfs_cd(a && *a ? a : "/"); }
static void cmd_cat(char* a)      { if (a && *a) vfs_cat(a); else kprint("Usage: cat <file>\n"); }
static void cmd_mkdir(char* a)    { if (a && *a) vfs_mkdir(a); else kprint("Usage: mkdir <dir>\n"); }
static void cmd_touch(char* a)    { if (a && *a) vfs_touch(a); else kprint("Usage: touch <file>\n"); }
static void cmd_echo(char* a)     { if (a) kprint(a); kputc('\n'); }
static void cmd_clear(char* a)    { (void)a; kclean_display(); }
static void cmd_uname(char* a)    { (void)a; kprint("Anaga x86 i686\n"); }
static void cmd_version(char* a)  { (void)a; print_kernel_version(); }
static void cmd_poweroff(char* a) { (void)a; system_shutdown(); }
static void cmd_run(char* a);
static void cmd_reboot(char* a)   { (void)a; system_reboot(); }

static const cmd_t cmds[] = {
    {"help",     cmd_help,     "Show help"},
    {"ls",       cmd_ls,       "List files"},
    {"pwd",      cmd_pwd,      "Print directory"},
    {"cd",       cmd_cd,       "Change directory"},
    {"cat",      cmd_cat,      "Show file"},
    {"mkdir",    cmd_mkdir,    "Create directory"},
    {"touch",    cmd_touch,    "Create file"},
    {"echo",     cmd_echo,     "Echo text"},
    {"clear",    cmd_clear,    "Clear screen"},
    {"uname",    cmd_uname,    "System info"},
    {"version",  cmd_version,  "Show version"},
    {"run",      cmd_run,      "Run ELF binary"},
    {"poweroff", cmd_poweroff, "Shutdown"},
    {"reboot",   cmd_reboot,   "Reboot"},
    {0, 0, 0}
};

extern void process_exec(uint32_t entry);

static void cmd_run(char* a) {
    if (!a || !*a) { kprint("Usage: run <path>\n"); return; }
    uint32_t entry;
    if (elf_load(a, &entry) == 0) {
        kprintf("Executing ELF at 0x%x...\n", entry);
        process_exec(entry);
    }
}

static void cmd_help(char* a) {
    (void)a;
    for (int i = 0; cmds[i].name; i++) {
        kprint("  ");
        kprint(cmds[i].name);
        int pad = 10 - strlen(cmds[i].name);
        while (pad-- > 0) kputc(' ');
        kprint("- ");
        kprint(cmds[i].desc);
        kputc('\n');
    }
}

static void parse(char* s, char** name, char** args) {
    *name = s;
    *args = 0;
    while (*s && *s != ' ') s++;
    if (*s == ' ') {
        *s++ = '\0';
        while (*s == ' ') s++;
        if (*s) *args = s;
    }
}

void handle_command(char* cmd) {
    if (!*cmd) return;

    char *name, *args;
    parse(cmd, &name, &args);

    for (int i = 0; cmds[i].name; i++) {
        if (strcmp(name, cmds[i].name) == 0) {
            cmds[i].fn(args);
            return;
        }
    }
    kprint(name);
    kprint(": not found\n");
}