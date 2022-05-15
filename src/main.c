//
// Created by Norman on 2021/9/24.
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/StrQueue.h"
#include "../include/downloader.h"

#define BUFFER_SIZE 512

enum FIELD_STATUS {
    FIELD_OTHERS, FIELD_PROXIES, FIELD_PROXY_GROUPS, FIELD_RULES
};

void file2Queues(char *filename, struct StrQueue *queue_proxies, struct StrQueue *queue_proxy_groups,
                 struct StrQueue *queue_rules);

void trim(char *str_in, char *str_out, int max_size);

char *trimHead(char *line_in);

void trim(char *str_in, char *str_out, int max_size) {

    int i, j;

    i = 0;

    j = (int) strlen(str_in) - 1;

    while ((str_in[i] == ' ') || (str_in[i] == '\t') || (str_in[i] == '\r'))
        ++i;

    while ((str_in[j] == ' ') || (str_in[j] == '\t') || (str_in[j] == '\r'))
        --j;

    if (j > max_size) j = max_size - 1;
    strncpy(str_out, str_in + i, j - i + 1);
    str_out[j - i + 1] = '\0';
}

char *trimHead(char *line_in) {
    int i = 0;
    while ((line_in[i] == ' ') || (line_in[i] == '\t') || (line_in[i] == '\r'))
        ++i;
    return line_in + i;
}

void file2Queues(char *filename, struct StrQueue *queue_proxies, struct StrQueue *queue_proxy_groups,
                 struct StrQueue *queue_rules) {
    FILE *fp_downloaded;
    int line_size = BUFFER_SIZE;
    int queue_status = FIELD_OTHERS;
    int last_queue_status = FIELD_OTHERS;

    char buffer[BUFFER_SIZE];
    char buffer_cmp[BUFFER_SIZE];
    char *line = (char *) malloc(sizeof(char) * line_size);
    if (line == NULL) {
        printf("Error allocate memory(buffer line)\n");
        exit(1);
    }
    int count_buffer = 0;

    /* opening file for reading */
    fp_downloaded = fopen(filename, "r");

    if (fp_downloaded == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    while (fgets(buffer, sizeof(buffer), fp_downloaded) != NULL) {
        strncpy(line, buffer, BUFFER_SIZE);
        while (strchr(buffer, '\n') == NULL) {
            count_buffer += 1;
            if (fgets(buffer, sizeof(buffer), fp_downloaded) == NULL)
                break;
            line_size += BUFFER_SIZE;
            line = realloc(line, line_size); // re allocate memory.
            // printf("%s\n", line);
            strncpy(line + (line_size - BUFFER_SIZE - count_buffer), buffer, BUFFER_SIZE);

        }

        /*
         * code for processing new line here.
         */

        // debug printf
        // printf("(line size=%d, str length=%lu)\n%s", line_size, strlen(line), line);
        // printf("%s", line);
        trim(line, buffer_cmp, BUFFER_SIZE);

        // printf("%d", strcmp(buffer_cmp, "proxy-groups:\n"));
        if (strcmp(buffer_cmp, "proxies:\n") == 0) {
            queue_status = FIELD_PROXIES;
            goto READ_NEXT;
        } else if (strcmp(buffer_cmp, "proxy-groups:\n") == 0) {
            queue_status = FIELD_PROXY_GROUPS;
            goto READ_NEXT;
        } else if (strcmp(buffer_cmp, "rules:\n") == 0) {
            queue_status = FIELD_RULES;
            goto READ_NEXT;
        } else if (strlen(line) > 4) {
            if (strchr(buffer_cmp, '{') == NULL && strchr(buffer_cmp, ':') != NULL) {
                queue_status = FIELD_OTHERS;
                goto READ_NEXT;
            }
        }

        switch (queue_status) {
            case FIELD_OTHERS:
                break;
            case FIELD_PROXIES:
                if (last_queue_status != FIELD_PROXIES) {
                    last_queue_status = FIELD_PROXIES;
                }
                enQueue(queue_proxies, line);
                break;
            case FIELD_PROXY_GROUPS:
                if (last_queue_status != FIELD_PROXY_GROUPS) {
                    last_queue_status = FIELD_PROXY_GROUPS;
                }
                enQueue(queue_proxy_groups, line);
                break;
            case FIELD_RULES:
                if (last_queue_status != FIELD_RULES) {
                    last_queue_status = FIELD_RULES;
                }
                enQueue(queue_rules, line);
                break;
            default:
                break;
        }
        READ_NEXT:
        count_buffer = 0;
        if (line_size != BUFFER_SIZE) line = realloc(line, line_size); // re allocate memory.
        line_size = BUFFER_SIZE;
    }

    /*
     * end here:
     */

    free(line);
    line = NULL;
    fclose(fp_downloaded);
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Bad args, need three arguments (in order): url user_rule_file output_file\n");
        return -1;
    }
    char *url = argv[1];
    char *user_file = argv[2];
    char *dst_file = argv[3];

    struct StrQueue *Q_proxies = NULL;
    struct StrQueue *Q_proxy_groups = NULL;
    struct StrQueue *Q_rules = NULL;

    printf("updating from subscription link...\n");
    download(url, "config.download.tmp");

    initQueue(&Q_proxies);
    initQueue(&Q_proxy_groups);
    initQueue(&Q_rules);

    file2Queues("config.download.tmp", Q_proxies, Q_proxy_groups, Q_rules);

    /*
     * process out file here
     * */

    printf("loaded:\t%d proxy(s), %d proxy-group(s), %d rule(s)\n", Q_proxies->count, Q_proxy_groups->count,
           Q_rules->count);

    FILE *fp_user, *fp_dst;
    int line_size = BUFFER_SIZE;
    char buffer[BUFFER_SIZE];
    char buffer_cmp[BUFFER_SIZE];
    char *line = (char *) malloc(sizeof(char) * line_size);
    char *queue_out = NULL;

    int count_proxies = 0;
    int count_proxy_groups = 0;
    int count_rules = 0;

    int count_proxies_u = 0;
    int count_proxy_groups_u = 0;
    int count_rules_u = 0;

    int count_empty_line = 0;

    int last_field = FIELD_OTHERS;
    int current_field = FIELD_OTHERS;
    int is_proxies_done = 0;
    int is_proxy_groups_done = 0;
    int is_rule_done = 0;

    if (line == NULL) {
        printf("Error allocate memory(buffer line)\n");
        exit(1);
    }
    int count_buffer = 0;

    /* opening file for reading */
    fp_user = fopen(user_file, "r");
    if (fp_user == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    fp_dst = fopen(dst_file, "w");
    if (fp_dst == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    printf("writing config to %s...\n\n", dst_file);
    while (fgets(buffer, sizeof(buffer), fp_user) != NULL) {
        strncpy(line, buffer, BUFFER_SIZE);
        while (strchr(buffer, '\n') == NULL) {
            count_buffer += 1;
            if (fgets(buffer, sizeof(buffer), fp_user) == NULL)
                break;
            line_size += BUFFER_SIZE;
            line = realloc(line, line_size); // re allocate memory.
            // printf("%s\n", line);
            strncpy(line + (line_size - BUFFER_SIZE - count_buffer), buffer, BUFFER_SIZE);

        }
        // printf("%s", line);
        trim(line, buffer_cmp, BUFFER_SIZE);

        //printf("%d", strcmp(buffer_cmp, "proxy-groups:\n"));
        if (strcmp(buffer_cmp, "proxies:\n") == 0) {
            last_field = current_field;
            current_field = FIELD_PROXIES;
        } else if (strcmp(buffer_cmp, "proxy-groups:\n") == 0) {
            last_field = current_field;
            current_field = FIELD_PROXY_GROUPS;
        } else if (strcmp(buffer_cmp, "rules:\n") == 0) {
            last_field = current_field;
            current_field = FIELD_RULES;
        } else if ((strchr(line, '{') == NULL) && (strchr(line, ':') != NULL)) {
            last_field = current_field;
            current_field = FIELD_OTHERS;
        }

        count_buffer = 0;
        if (line_size != BUFFER_SIZE) line = realloc(line, line_size); // re allocate memory.
        line_size = BUFFER_SIZE;

        switch (last_field) {
            case FIELD_PROXIES:
                if (last_field == current_field) {
                    fprintf(fp_dst, "  %s", trimHead(line));
                    if (strchr(line, '-')) count_proxies_u += 1;
                } else {
                    fprintf(fp_dst, "\n  # subscribed proxies:\n");
                    while (Q_proxies->front != Q_proxies->rear) {
                        deQueue(Q_proxies, &queue_out);
                        // printf("%s", queue_out);
                        fprintf(fp_dst, "  %s", trimHead(queue_out));
                        if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
                        if (strchr(queue_out, '-')) count_proxies += 1;
                        free(queue_out);
                        queue_out = NULL;
                    }
                    fprintf(fp_dst, "%s", line);
                    is_proxies_done = 1;
                    last_field = current_field;
                }
                break;
            case FIELD_PROXY_GROUPS:
                if (last_field == current_field) {
                    fprintf(fp_dst, "  %s", trimHead(line));
                    if (strchr(line, '-')) count_proxy_groups_u += 1;
                } else {
                    fprintf(fp_dst, "\n  # subscribed proxy-groups:\n");
                    while (Q_proxy_groups->front != Q_proxy_groups->rear) {
                        deQueue(Q_proxy_groups, &queue_out);
                        // printf("%s", queue_out);
                        fprintf(fp_dst, "  %s", trimHead(queue_out));
                        if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
                        if (strchr(queue_out, '-')) count_proxy_groups += 1;
                        free(queue_out);
                        queue_out = NULL;
                    }
                    fprintf(fp_dst, "%s", line);
                    is_proxy_groups_done = 1;
                    last_field = current_field;
                }
                break;
            case FIELD_RULES:
                if (last_field == current_field) {
                    fprintf(fp_dst, "  %s", trimHead(line));
                    if (strchr(line, '-')) count_rules_u += 1;
                } else {
                    fprintf(fp_dst, "\n  # subscribed rules:\n");
                    while (Q_rules->front != Q_rules->rear) {
                        deQueue(Q_rules, &queue_out);
                        // printf("%s", queue_out);
                        fprintf(fp_dst, "  %s", trimHead(queue_out));
                        if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
                        if (strchr(queue_out, '-')) count_rules += 1;
                        free(queue_out);
                        queue_out = NULL;
                    }
                    fprintf(fp_dst, "%s", line);
                    is_rule_done = 1;
                    last_field = current_field;
                }
                break;
            case FIELD_OTHERS:
                fprintf(fp_dst, "%s", line);
            default:
                break;
        }
        /*
         * end
         * */
    }

    // write to file when EOF and still got data.
    if (is_proxies_done == 0) {
        fprintf(fp_dst, "\n  # subscribed proxies:\n");
        while (Q_proxies->front != Q_proxies->rear) {
            deQueue(Q_proxies, &queue_out);
            // printf("%s", queue_out);
            fprintf(fp_dst, "  %s", trimHead(queue_out));
            if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
            if (strchr(queue_out, '-')) count_proxies += 1;
            free(queue_out);
            queue_out = NULL;
        }
    }
    if (is_proxy_groups_done == 0) {
        while (Q_proxy_groups->front != Q_proxy_groups->rear) {
            deQueue(Q_proxy_groups, &queue_out);
            // printf("%s", queue_out);
            fprintf(fp_dst, "  %s", trimHead(queue_out));
            if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
            if (strchr(queue_out, '-')) count_proxy_groups += 1;
            free(queue_out);
            queue_out = NULL;
        }
    }
    if (is_rule_done == 0) {
        fprintf(fp_dst, "\n  # subscribed rules:\n");
        while (Q_rules->front != Q_rules->rear) {
            deQueue(Q_rules, &queue_out);
            // printf("%s", queue_out);
            fprintf(fp_dst, "  %s", trimHead(queue_out));
            if (strchr(queue_out, '\n') == NULL)fprintf(fp_dst, "\n");
            if (strchr(queue_out, '-')) count_rules += 1;
            free(queue_out);
            queue_out = NULL;
        }
    }
    if ((Q_rules->front != Q_rules->rear) || (Q_rules->count != 0))
        printf("writing config (rules) incomplete, please report bug?\n");
    if ((Q_proxies->front != Q_proxies->rear) || Q_proxies->count != 0)
        printf("writing config (proxies) incomplete, please report bug?\n");
    if ((Q_proxy_groups->front != Q_proxy_groups->rear) || Q_proxy_groups->count != 0)
        printf("writing config (proxy-groups) incomplete, please report bug?\n");

    printf("               \tuser\tlink\tout\n"
           "proxy(s):      \t%d\t+\t%d\t->\t%d\n"
           "proxy-group(s):\t%d\t+\t%d\t->\t%d\n"
           "rules:         \t%d\t+\t%d\t->\t%d\n\n",
           count_proxies_u, count_proxies, count_proxies_u + count_proxies,
           count_proxy_groups_u, count_proxy_groups, count_proxy_groups_u + count_proxy_groups,
           count_rules_u, count_rules, count_rules_u + count_rules);

    // free res
    fclose(fp_user);
    fclose(fp_dst);
    deleteQueue(&Q_proxies);
    deleteQueue(&Q_proxy_groups);
    deleteQueue(&Q_rules);
    free(line);
}