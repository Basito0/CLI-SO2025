#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

//Esta función hace lo que dice: extrae toda la línea de comando de la terminal
char* GetCommandLine(){
    unsigned int len_max = 128;
    unsigned int current_size = 0;
    char* pStr = malloc(len_max);
    current_size = len_max;

    if(pStr != NULL)
    {
    int c = EOF;
    unsigned int i = 0;
    while (( c = getchar() ) != '\n' && c != EOF)
    {
        pStr[i++] = (char) c;
        if(i == current_size)
        {
            current_size = i+len_max;
            pStr = realloc(pStr, current_size);
        }
    }
    }
    return pStr;
}

//Cuenta cuantos "vertical bar" hay en el string s
int CountPipes(char* s){
    int pipe_count = 0;
    for(int i = 0; i<strlen(s); i++){
        if(s[i] == '|') pipe_count++;
    }
    return pipe_count;
}

//el string está vacío?
bool IsStringEmpty(char* s){

    int word_len = 0;
    if (s[0] == '\0'){
        return true;
    }
    if (s[0] != ' '){
        return false;
    }
    while(s[word_len]!= '\0'){
        word_len++;
        if (s[word_len] == ' '){
            continue;
        }
        else if (s[word_len] == '\0'){
            return true;
        }
        else{ return false;}
    }
    return false;
}

//se parece a isStringEmpty pero incluye "vertical bar" para comparación
bool AreArgumentsLeft(char* s){
    int i = 0;
    if (s[i] == '\0'){
        return false;
    }
    if (s[i] == '|'){
        return false;
    }
    while(s[i] != '\0' && s[i] != '|'){
        if (s[i] == ' '){
            i++;
            continue;
        }
        else{
            return true;
        }
    }
    return false;
}

//Entrega la primera palabra que encuentra en el string s sin incluir "vertical bar"
//si no hay absolutamente nada después de la palabra entonces pair[1] va a ser un '\0'
char** ExtractFirstWord(char* s){
    int bar_index = strcspn(s, "|");
    int first_word_end_index = 0;
    bool counting_word = false;
    char* word = (char*) malloc(1024);
    int word_len = 0;

    for (int i = 0; i < bar_index; i++)
    {
        if (s[i] == ' ' && !counting_word)
        {
            continue;
        }
        else if (s[i] == ' ' && counting_word)
        {
            first_word_end_index = i;
            break;
        }
        else if (s[i] == '\0' && counting_word)
        {
            //printf("null encontrado");
            first_word_end_index = i;
            break;
        }


        counting_word = true;
        word[word_len++] = s[i];
        first_word_end_index = i;
    }

    word[word_len] = '\0';

    int new_array_size = strlen(s) - first_word_end_index;
    char* new_s = malloc(new_array_size+1);

    for (int i = 0; i < new_array_size;)
    {
        if(i == (new_array_size-1)){
            new_s[i] = '\0';
            break;
        }

        new_s[i] = s[first_word_end_index+1];
        i++;
        first_word_end_index++;
    }



    char** pair = malloc(2 * sizeof(char*));
    pair[0] = malloc(strlen(word) + 1);
    pair[1] = malloc(strlen(new_s) + 1);
    strcpy(pair[0], word);
    strcpy(pair[1], new_s);

    return pair;
}

/*Esta función se encarga de devolver un array con el comando y los argumentos.
el segundo elemento es el resto del comando*/

char** GetCommandArguments(char* s){
    char* string;
    char** args1;
    char** args2;

    //Aquí se extrae el comando y se guarda en args1
    if (AreArgumentsLeft(s))
    {
        char** words = ExtractFirstWord(s);
        
        string = (char*) malloc(strlen(words[1]) + 1);

        strcpy(string, words[1]);

        args1 = malloc(1 * sizeof(char*));
        args1[0] = (char*) malloc(strlen(words[0]) + 1);
        strcpy(args1[0], words[0]);

        //esta parte es para pegar el resto del string en args1
        if(strlen(string) > 0){
            args1 = realloc(args1, 2*(sizeof(char*)));
            args1[0] = (char*) malloc(strlen(words[0]) + 1);
            args1[1] = (char*) malloc(strlen(string) + 1);
            strcpy(args1[0], words[0]);
            strcpy(args1[1], string);
        }
    }
    else{
        //Si llegaste acá es porque no pasaste ningún comando.
        args1 = malloc(1 * sizeof(char*));
        args1[0] = malloc(sizeof(NULL));
        args1[1] = malloc(sizeof(s));
        strcpy(args1[1], s);
        args1[0] = NULL;
        return args1;
    }

    //si no hay flags mandamos el comando y el resto de la línea de vuelta
    if (!AreArgumentsLeft(string))
    {
        args2 = malloc(2 * sizeof(char*) );
        args2[0] = (char*) malloc(strlen(args1[0]) + 1);

        if(strlen(string) > 0){
            args2[1] = (char*) malloc(strlen(string) + 1);
            strcpy(args2[1], string);
        }
        else{ args2[1] = NULL; }

        strcpy(args2[0], args1[0]);
        return args2;
    }


    int i = 2;
    //Ahora creamos un vector con argumentos
    while(AreArgumentsLeft(string)){
        char** words = ExtractFirstWord(string);
        free(string);
        
        string = (char*) malloc(strlen(words[1]) + 1);
        strcpy(string, words[1]);

        args2 = malloc((1 + i) * sizeof(char*));

        for(int j = 0; j < i; j++){
            args2[j] = (char*) malloc(strlen(args1[j]) + 1);
            strcpy(args2[j], args1[j]);
        }

        args2[i] = (char*) malloc(strlen(words[0]) + 1);
        strcpy(args2[i], words[0]);
        free(args1);

        //Devolvemos las cosas a args1
        args1 = malloc((1 + i) * sizeof(char*));

        for(int j = 0; j <= i; j++){
            args1[j] = (char*) malloc(strlen(args2[j]) + 1);
            strcpy(args1[j], args2[j]);
        }
        free(args2);

        //esta parte es para pegar el resto del string en args2
        if(strlen(string) > 0){
            args1[1] = (char*) realloc(args1[1], strlen(string));
            strcpy(args1[1], string);
        }
        else{
            args1[1] = NULL;
        }

        i++;
    }

    args2 = malloc((i) * sizeof(char*));

    for(int j = 0; j < i; j++){
        if(j == 1){continue;}
        args2[j] = (char*) malloc(strlen(args1[j]) + 1);
        strcpy(args2[j], args1[j]);
    }
    free(args1);
    args2[i] = NULL;

    //esta parte es para pegar el resto del string en args2
    if(strlen(string) > 0){
        args2[1] = (char*) realloc(args2[1], strlen(string));
        strcpy(args2[1], string);
    }
    else{
        args2[1] = NULL;
    }

    return args2;
}

//Es para borrar los vertical bar (|) que pueden sobrar después de usar ExtractFirstWord.
char* ErasePipe(char* s){
    if (s == NULL)
    {
        return NULL;
    }
    if(CountPipes(s) == 0){
        return s;
    }

    int bar_index = strcspn(s, "|");
    if (bar_index == strlen(s))
    {
        return s;
    }
    char* new_s = realloc(s, strlen(s));
    new_s[bar_index] = ' ';

    return new_s;
}

//este vector sirve para ordenar el vector de args_array y organizarlo para tener el formato de la system call
//o sea, comando -> flag1 -> flag2
char** GetArgsVector(char** args_array){

    char** args;

    //tamaño del array
    int i = 0;
    //si hay flags, entonces args_array[2] es la flag e i es al menos 3
    if (args_array[2] != NULL)
    {
        //printf("hay más de 2 elementos\n");
        i = 2;
        while(args_array[i] != NULL){
            i++;
        }
    }
    else{i=2;}

    args = malloc((i + 1) * sizeof(char*));

    //char* argc = args_array[0];
    //char** argv;

    args[0] = (char*) malloc( strlen(args_array[0]) + 1 );
    strcpy(args[0], args_array[0]);
    //printf("el comando es '%s'\n", args[0]);

    //si aparte del comando y el resto hay flags entonces agregar a argv
    if(i>2){
        //argv = malloc( (i-2) * sizeof(char*) );
        for (int j = 2; j < i; j++)
        {
            //esto es por si se necesita argv
            /*
            argv[j-2] = malloc(strlen(args_array[j]));
            strcpy(argv[j-2], args_array[j]);
            */

            //esto es para args
            args[j-1] = malloc(strlen(args_array[j]) + 1);
            strcpy(args[j-1], args_array[j]);
            //printf("un argumento es '%s'", args[j-1]);
        }
    }
    args[i-1] = malloc(sizeof(NULL));
    args[i-1] = NULL;
    //printf("el anterior al final es %s\n", args[i-1]);

    //Verificación
    /*
    printf("El comando es: %s\n", argc);
    printf("Los argumentos son: \n");
    if (i > 3)
    {
        for (int j = 2; j < i; j++)
        {
            printf("%s\n", argv[j-2]);
        }
    }
    */

    //free(argc);
    //free(argv);

    return args;
}

char GetNextCharacter(char* s){
    if(s == NULL){
        return '\0';
    }
    for (int i = 0; i < strlen(s); ++i)
    {
        if(s[i] == ' '){
            continue;
        }
        else if(s[i] == '|'){
            return '|';
        }
        else if(s[i] == '\0'){
            return '\0';
        }
        else{
            return s[i];
        }
    }
    return '\0';
}

int main()
{
    //While 1 para pedir comando todo el rato
    restart_loop:
    while(1){
        char* commandline;
        printf("mishell:$ ");
        commandline = GetCommandLine();
        int command_number = CountPipes(commandline);
        char*** command_array;
        command_array = malloc(sizeof(char**));

        for (int i = 0; i <= command_number; i++)
        {
            //printf("pasando al siguiente comando\n");
            char** args_array = GetCommandArguments(commandline);   //usar esto para obtener el resto del comando (args_array[1])
            //printf("comandos obtenidos\n");

            //esto es por si el comando está vacío y hay un pipe delante
            if (args_array[0] == NULL && args_array[1] != NULL && GetNextCharacter(args_array[1]) == '|')
            {
                printf("bash: syntax error near unexpected token '%c'\n", GetNextCharacter(args_array[1]));
                free(commandline);
                free(args_array);
                free(command_array);
                goto restart_loop;
            }
            else if(args_array[0] == NULL && (GetNextCharacter(args_array[1]) == ' ' || GetNextCharacter(args_array[1]) == '\0' || args_array[1] == NULL)){
                printf(">");
                commandline = GetCommandLine();
                int to_add = CountPipes(commandline);
                command_number += to_add;
                //printf("la nueva línea de comandos restantes es: %s\n", commandline);
                args_array = GetCommandArguments(commandline);
            }

            char* remaining_commands = args_array[1];

            args_array = GetArgsVector(args_array);                              //esto es para tener el arreglo en el formato c, flag1, flag2...

            if(remaining_commands != NULL){
                remaining_commands = ErasePipe(remaining_commands);

                if(remaining_commands == NULL){
                    commandline = NULL;
                    //printf("el resto es nulo\n");
                    //printf("el resto del string es: %s\n", commandline);
                    free(remaining_commands);
                    command_array[i] = args_array;
                    continue;
                }

                commandline = strdup(remaining_commands);
                free(remaining_commands);
                //printf("el resto del string es: %s\n", commandline);
            }else{
                commandline = NULL;
            }

            command_array[i] = args_array;
        }

        //esta es la parte donde se hacen procesos hijos y se asignan pipes y todo eso
        //STDIN_FILENO es 0
        //STDOUT_FILENO es 1
        int pipes[2 * command_number];
        for (int i = 0; i < command_number; i++) {
            if (pipe(pipes + 2*i) < 0) {
                perror("pipe");
                exit(1);
            }
        }

        for (int i = 0; i <= command_number; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }
            if (pid == 0) {
                if (i > 0) {
                    dup2(pipes[2*(i-1)], STDIN_FILENO);
                }
                if (i < command_number) {
                    dup2(pipes[2*i + 1], STDOUT_FILENO);
                }
                for (int j = 0; j < 2*command_number; j++) {
                    close(pipes[j]);
                }
                execvp(command_array[i][0], command_array[i]);
                perror(command_array[i][0]);
                exit(1);
            }
        }

        for (int j = 0; j < 2*command_number; j++) {
            close(pipes[j]);
        }
        for (int i = 0; i <= command_number; i++) {
            wait(NULL);
        }

        // free and loop back for next prompt
        free(commandline);
        free(command_array);
        goto restart_loop;
    }

    return 0;
}
