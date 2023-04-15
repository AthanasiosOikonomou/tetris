/* Author: Oikonomou Athanasios
 *
 * Created on November 26, 2012, 12:54 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define LIB 133

//*******************************VARIABLES**************************************

//int num_gates[23] = {27, 208, 298, 344, 349, 382, 386, 400, 420, 444, 510, 526, 641, 713, 820, 832, 838, 953, 1196, 1238, 1423, 1488, 1494};
float cellwidth = 0.0;
float cellheight = 1.4;
float chipwidth = 0.0;
float chipheigth = 0.0;
FILE* fcells;
FILE* fnets;
FILE* flib;
FILE* fdchip;
FILE* fresults;
float wirelenght = 0.0;
float widthest = 0.0;
int id_temp = 0;

//*******************************STRUCTS****************************************

struct gate {
    int id;
    char gate_type[10];
    float x;
    float y;
    float width;
    char net_gate[20];
    char net[100];
    struct gate *next;
    int placed;
};

struct library {
    char gate_type[20];
    double width;
    double height;
};

//******************************FUNCTIONS***************************************

float effective_x(float, float, float);
double tetris(struct gate *);
float cost(struct gate *, struct gate *, int, int);

//********************************MAIN******************************************

int main(int argc, char * argv[]) {

    printf("--STARTING PROGRAMM\n");
    struct gate *current, *head;
    struct library lib[LIB];
    head = (struct gate *) calloc(1, sizeof (struct gate));

    //file opening...
    printf("--FILE OPENING\n");
    fcells = fopen("data/unlegalized_cells/S27.txt", "r");
    fnets = fopen("data/nets/S27.txt", "r");
    flib = fopen("data/NangateLibrarySizes.txt", "r");
    fdchip = fopen("data/dimensions/S27.txt", "r");
    fresults = fopen("data/my_results.txt", "w");
    if (fcells != 0 && fnets != 0 && flib != 0 && fdchip != 0 && fresults != 0) printf("----THE FILES HAS BEEN SUCCESSFULLY OPEN\n");

    //data reading...
    printf("--DATA READING\n");
    current = head;
    int o;
    char temp;
    char* stop;

    //_____chip____
    printf("----CHIP\n");
    fscanf(fdchip, "chip height = %f chip width = %f", &chipheigth, &chipwidth);

    //______library______
    printf("----LIBRARY\n");
    int w = 0;
    temp = fgetc(flib);
    while (temp != 'Y') temp = fgetc(flib); // throw headers
    temp = fgetc(flib); //throw new line
    temp = fgetc(flib); //throw new line
    do {
        //finding gate type
        temp = fgetc(flib);
        o = 0;
        while (temp != ' ') {
            lib[w].gate_type[o++] = temp;
            temp = fgetc(flib);
        }
        lib[w].gate_type[o++] = '\0';

        //finding dimensions
        fscanf(flib, "%lf %lf", &lib[w].width, &lib[w].height);
        temp = fgetc(flib); //throw new line
        w++;
    } while (w < LIB);

    //_____cells____
    printf("----CELLS\n");
    do {
        //setting id
        current->id = id_temp;
        //finding gate type
        temp = fgetc(fcells);
        o = 0;
        while (temp != '\t') {
            current->gate_type[o++] = temp;
            temp = fgetc(fcells);
        }
        current->gate_type[o++] = '\0';

        //inputs and outputs cant moved during placement
        if (strcmp(current->gate_type, "input") == 0 || strcmp(current->gate_type, "output") == 0) current->placed = 1;
        while (temp == '\t') {
            temp = fgetc(fcells);
        }

        //finding net gate point
        o = 0;
        temp = fgetc(fcells);
        while (temp != '\t') {
            current->net_gate[o++] = temp;
            temp = fgetc(fcells);
        }
        current->net_gate[o++] = '\0';
        stop = current->net_gate;

        //finding gate position
        fscanf(fcells, "%f %f", &current->x, &current->y);
        temp = fgetc(fcells); //throw new line

        //finding width and bigger width
        for (w = 0; w < LIB; w++) {
            if (strcmp(current->gate_type, lib[w].gate_type) == 0)
                current->width = lib[w].width;
        }
        if (current->width > widthest) widthest = current->width;
        current->net[0] = '\0';

        //new gate
        if (strcmp(stop, "1") != 0) {
            current->next = (struct gate *) calloc(1, sizeof (struct gate));
            current = current->next;
        }
        id_temp++;
    } while (strcmp(stop, "1") != 0);


    //______net list_____
    printf("----NETS\n");
    char temp_net[20], buffer[20];
    int temp_num;
    int new_net = 1;
    temp = fgetc(fnets);

    do {
        if (new_net == 1) {
            temp_net[0] = '\0';
            o = 0;
            while (temp != '\t') {
                temp_net[o++] = temp;
                temp = fgetc(fnets);
            }
            temp_net[o++] = '\0';
            new_net = 0;
        }

        if (temp == 'U') {
            buffer[0] = '\0';
            fscanf(fnets, "%d", &temp_num);
            sprintf(buffer, "%d", temp_num);
            current = head;
            while (current != NULL) {
                if (strcmp(current->gate_type, "output") != 0 && strcmp(current->gate_type, "input") != 0)
                    if (strcmp(current->net_gate, buffer) == 0) {
                        strcat(current->net, temp_net);
                        strcat(current->net, ".\0");
                    }
                current = current->next;
            }
        }
        if (temp == 'S') {
            buffer[0] = '\0';
            fscanf(fnets, "%d", &temp_num);
            sprintf(buffer, "%d", temp_num);
            current = head;
            while (current != NULL) {
                if (strcmp(current->gate_type, "output") == 0 || strcmp(current->gate_type, "input") == 0)
                    if (strcmp(current->net_gate, buffer) == 0) {
                        strcat(current->net, temp_net);
                        strcat(current->net, ".\0");
                    }
                current = current->next;
            }
        }
        if (temp == ';') new_net = 1;
        temp = fgetc(fnets);
        if (temp == '\n') temp = fgetc(fnets);
    } while (temp != EOF);

    //data processing...
    printf("--TETRIS ALGORITHM\n");
    printf("----STARTING\n");
    double total_wirelength = 0.0;
    total_wirelength = tetris(head);



    //data results...
    printf("--RESULTS\n");
    printf("----TOTAL WIRE LENGTH: %f\n", total_wirelength);
    printf("----EXPORTING TO FILE\n");
    printf("----CLOSING FILES\n");
    fclose(fcells);
    fclose(fnets);
    fclose(flib);
    fclose(fdchip);
    fclose(fresults);
    return 0;

}


//***************************FUNCTIONS******************************************       

float effective_x(float x, float width, float factor) {
    return x - width*factor;
}

float cost(struct gate *head, struct gate *current, int target_x, int target_y) {
    float max_x = 0.0, min_x = 0.0, max_y = 0.0, min_y = 0.0;
    float wire_cost = 0.0;
    struct gate *current3;
    int x = 0, y = 0;
    char current_net[10];

    max_x = min_x = target_x;
    max_y = min_y = target_y;
    current_net[0] = '\0';
    while (current->net[y] != '\0') {
        if (current->net[y] == '.') {
            current_net[x] = '\0';
            x = 0;
            y++;

            //for each net current gate belongs
            current3 = head;
            while (current3 != NULL) {

                //find cells in the same net
                if (strstr(current3->net, current_net) != NULL) {
                    if (current3->x > max_x) max_x = current3->x;
                    if (current3->x < min_x) min_x = current3->x;
                    if (current3->y > max_y) max_y = current3->y;
                    if (current3->y < min_y) min_y = current3->y;
                }
                current3 = current3->next;
            }
            wire_cost = wire_cost + max_x - min_x + max_y - min_y;
            current_net[0] = '\0';
        } else {
            current_net[x] = current->net[y];
            x++;
            y++;
        }
    }
    return wire_cost;
}

double tetris(struct gate * head) {
    float width_factor = 0.5;
    float total_wire = 0.0;

    //new x coordinate 
    struct gate *current, *current2;
    current = head;
    while (current != NULL) {
        current->x = effective_x(current->x, current->width, width_factor);
        current = current->next;
    }

    //sort cells    
    for (current = head; current != NULL; current = current->next) {
        for (current2 = current->next; current2 != NULL; current2 = current2->next) {
            if (current->x > current2->x) {
                int id1 = current->id, id2 = current2->id;
                current->id = id2;
                current2->id = id1;
                int placed1 = current->placed, placed2 = current2->placed;
                current->placed = placed2;
                current2->placed = placed1;
                char tempgate_type[10];
                strcpy(tempgate_type, current->gate_type);
                strcpy(current->gate_type, current2->gate_type);
                strcpy(current2->gate_type, tempgate_type);
                char tempnet[10];
                strcpy(tempnet, current->net);
                strcpy(current->net, current2->net);
                strcpy(current2->net, tempnet);
                float x1 = current->x;
                float x2 = current2->x;
                current->x = x2;
                current2->x = x1;
                float y1 = current->y, y2 = current2->y;
                current->y = y2;
                current2->y = y1;
                float width1 = current->width, width2 = current2->width;
                current->width = width1;
                current2->width = width2;
                char tempnet_gate[10];
                strcpy(tempnet_gate, current->net_gate);
                strcpy(current->net_gate, current2->net_gate);
                strcpy(current2->net_gate, tempnet_gate);
            }
        }
    }

    //create number of rows
    int num_rows = floor(chipheigth / head->width);
    float rows[num_rows];
    for (int i = 0; i < num_rows; i++)
        rows[i] = 0.0;

    //find new position and place for each cell
    float target_x = 0.0;
    float target_y = 0.0;
    int target_row_up = -1;
    int target_row_mid = -1;
    int target_row_down = -1;
    int goto_row = 0;
    float mid_cost = -0.1, up_cost = -0.1, down_cost = -0.1, min_cost = 0.0;


    current = head;
    while (current != NULL) {
        if (current->placed == 0) {
            target_row_up = -1;
            target_row_mid = -1;
            target_row_down = -1;
            mid_cost = -0.1;
            up_cost = -0.1;
            down_cost = -0.1;

            //target rows
            target_row_mid = floor(current->y / cellheight);
            if (target_row_mid > 0) target_row_down = target_row_mid - 1;
            if (target_row_mid < num_rows) target_row_up = target_row_mid + 1;

            //find cost for each possible row and store minimum
            if (target_row_mid != -1 && rows[target_row_mid] <= chipwidth - current->width / 2) {
                //target x y
                target_y = target_row_mid * cellheight + cellheight / 2;
                target_x = rows[target_row_mid] + current->width / 2;
                mid_cost = cost(head, current, target_x, target_y);
            }
            if (target_row_up != -1 && rows[target_row_up] <= chipwidth - current->width / 2) {
                target_y = target_row_up * cellheight + cellheight / 2;
                target_x = rows[target_row_up] + current->width / 2;
                up_cost = cost(head, current, target_x, target_y);
            }
            if (target_row_down != -1 && rows[target_row_down] <= chipwidth - current->width / 2) {
                target_y = target_row_down * cellheight + cellheight / 2;
                target_x = rows[target_row_down] + current->width / 2;
                down_cost = cost(head, current, target_x, target_y);
            }
            if (mid_cost > 0)
                min_cost = mid_cost;
            if (up_cost > 0)
                min_cost = up_cost;
            if (down_cost > 0)
                min_cost = down_cost;

            if (mid_cost > 0 && mid_cost < min_cost) {
                min_cost = mid_cost;
                goto_row = target_row_mid;
            }
            if (up_cost > 0 && up_cost < min_cost) {
                min_cost = up_cost;
                goto_row = target_row_up;
            }
            if (down_cost > 0 && down_cost < min_cost) {
                min_cost = down_cost;
                goto_row = target_row_down;
            }

            //final cell placement
            rows[goto_row] = rows[goto_row] + current->width;
            current->x = target_x;
            current->y = target_y;
            current->placed = 1;
        }

        current = current->next;
    }

    //find total wire length 
    float max_x = 0.0, min_x = 0.0, max_y = 0.0, min_y = 0.0;
    int x = 0, y = 0;
    char current_net[10];
    char nets_checked[10000];
    nets_checked[0] = '\0';

    current = head;
    while (current != NULL) {
        max_x = min_x = current->x;
        max_y = min_y = current->y;
        x=0;
        y=0;
        while (current->net[y] != '\0') {

            //next net found
            if (current->net[y] == '.') {
                current_net[x] = '\0';
                x = 0;
                y++;
                
                //find gates in the net 
                printf("wire for net: %s\n",current_net);
                current2 = head;
                if (strstr(nets_checked, current_net) == NULL) {
                    
                    //first time in net
                    //find net wire
                    while (current2 != NULL) {

                        //find cells in the same net
                        if (strstr(current2->net, current_net) != NULL) {printf("  cell in net: %s %s\n",current2->gate_type, current2->net_gate);
                            if (current2->x > max_x) max_x = current2->x;
                            if (current2->x < min_x) min_x = current2->x;
                            if (current2->y > max_y) max_y = current2->y;
                            if (current2->y < min_y) min_y = current2->y;
                        }
                        current2 = current2->next;
                    }
                    strcat(nets_checked, current_net);
                    strcat(nets_checked, "\0");
                    strcat(nets_checked, ".");
                    strcat(nets_checked, "\0");
                    printf("---%s\n",nets_checked);
                    total_wire = total_wire + max_x - min_x + max_y - min_y;
                    current_net[0] = '\0';
                }
            } else {
                current_net[x] = current->net[y];
                x++;
                y++;
            }
        }
        current = current->next;
    }

    return total_wire;
}