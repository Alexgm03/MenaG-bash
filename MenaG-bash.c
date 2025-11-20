#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>


#define MAX_CMD 256
#define MAX_ARGS 20

// Reap child processes (evita zombies)
void handle_sigchld(int) { while (waitpid(-1, NULL, WNOHANG) > 0); }

// Ejecuta comando externo con redirecciones y background
void execute_external(char *args[], int background, char *infile, char *outfile) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }

    if (pid == 0) {
        // hijo: configurar redirecciones si las hay
        if (infile) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) { perror("open infile"); exit(1); }
            dup2(fd, STDIN_FILENO); close(fd);
        }
        if (outfile) {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open outfile"); exit(1); }
            dup2(fd, STDOUT_FILENO); close(fd);
        }
        execvp(args[0], args);
        perror("execvp"); exit(1);
    } else {
        // padre
        if (!background) waitpid(pid, NULL, 0);
    }
}

// Simple builtins útiles
int builtin_cd(char *path) { return chdir(path ? path : getenv("HOME")); }
void builtin_pwd() { char cwd[1024]; if (getcwd(cwd, sizeof(cwd))) puts(cwd); }
void builtin_mkdir(char *name) { if (!name) fputs("mkdir: missing\n", stderr); else if (mkdir(name,0755)) perror("mkdir"); }
void builtin_rm(char *name) { if (!name) fputs("rm: missing\n", stderr); else if (remove(name)) perror("rm"); }
void builtin_cat(char *file) {
    if (!file) { fputs("cat: missing\n", stderr); return; }
    int fd = open(file, O_RDONLY);
    if (fd < 0) { perror("cat"); return; }
    char buf[1024]; ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) write(STDOUT_FILENO, buf, n);
    close(fd);
}
void builtin_cp(char *src, char *dst) {
    if (!src || !dst) { fputs("cp: missing\n", stderr); return; }
    int in = open(src, O_RDONLY);
    if (in < 0) { perror("cp"); return; }
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { perror("cp"); close(in); return; }
    char buf[1024]; ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) write(out, buf, n);
    close(in); close(out);
}
int builtin_mv(char *src, char *dst) {
    if (!src || !dst) {
        printf("mv: missing operand\n");
        return 1;
    }
    struct stat st;
    // ¿dst es un directorio?
    if (stat(dst, &st) == 0 && S_ISDIR(st.st_mode)) {

        // construir la ruta final: dst + "/" + nombre del archivo original
        char newpath[512];
        snprintf(newpath, sizeof(newpath), "%s/%s", dst, src);

        if (rename(src, newpath) != 0)
            perror("mv");

    } else {
        // mv normal: renombrar
        if (rename(src, dst) != 0)
            perror("mv");
    }

    return 1;
}

// Separa por espacios, detecta < > &
int parse_command(char *input, char *args[], int *background, char **infile, char **outfile) {
    *background = 0; *infile = *outfile = NULL;
    int argc = 0;
    char *tok = strtok(input, " ");
    while (tok && argc < MAX_ARGS-1) {
        if (!strcmp(tok, "&")) *background = 1;
        else if (!strcmp(tok, "<")) { tok = strtok(NULL, " "); if (tok) *infile = tok; }
        else if (!strcmp(tok, ">")) { tok = strtok(NULL, " "); if (tok) *outfile = tok; }
        else args[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    args[argc] = NULL;
    return argc;
}
// Main
int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    char line[MAX_CMD], *args[MAX_ARGS];
    while (1) {
        printf("MenaG-bash> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) { putchar('\n'); continue; }
        line[strcspn(line, "\n")] = 0;                 // quitar '\n'
        if (line[0] == 0) continue;

        int background; char *infile, *outfile;
        parse_command(line, args, &background, &infile, &outfile);
        if (!args[0]) continue;

        // Comandos internos (Builtins)
        if (!strcmp(args[0], "exit")) break;
        if (!strcmp(args[0], "cd")) { builtin_cd(args[1]); continue; }
        if (!strcmp(args[0], "pwd")) { builtin_pwd(); continue; }
        if (!strcmp(args[0], "mkdir")) { builtin_mkdir(args[1]); continue; }
        if (!strcmp(args[0], "rm")) { builtin_rm(args[1]); continue; }
        if (!strcmp(args[0], "cp")) { builtin_cp(args[1], args[2]); continue; }
        if (!strcmp(args[0], "mv")) { builtin_mv(args[1], args[2]); continue; }
        if (!strcmp(args[0], "cat") && args[1] != NULL) { builtin_cat(args[1]); continue; }
        // comando externo
        execute_external(args, background, infile, outfile);
    }

    return 0;
}
