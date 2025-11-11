#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Adj {
    size_t vertexId;
    struct Adj* next;
} _Adj, *Adj;

typedef Adj* Graph;

typedef enum Color {
    WHITE,
    GREY,
    BLACK
} Color;

Adj dfs(size_t vertexId, Graph graph, Color *vertexColors, Adj answer, bool *cycled) {
    if (*cycled || vertexColors[vertexId] == BLACK) return answer;

    if (vertexColors[vertexId] == GREY) {
        *cycled = true;
        return answer;
    }

    vertexColors[vertexId] = GREY;
    
    for (Adj it = graph[vertexId]; it != NULL; it = it->next) {
        answer = dfs(it->vertexId, graph, vertexColors, answer, cycled);
    }

    vertexColors[vertexId] = BLACK;

    Adj adj = (Adj)malloc(sizeof(_Adj));
    adj->vertexId = vertexId;
    adj->next = answer;

    return adj;
}

Adj sort(size_t n, Graph graph) {
    Adj answer = NULL;

    Color *vertexColors = (Color*)malloc(sizeof(Color) * n);

    for (size_t i = 0; i < n; i++) {
        vertexColors[i] = WHITE;
    }

    bool cycled = false;
    for (size_t i = 0; i < n; i++) {
        answer = dfs(i, graph, vertexColors, answer, &cycled);
        if (cycled) break;
    }

    free(vertexColors);

    if (cycled) {
        while (answer != NULL) {
            Adj adj = answer;
            answer = answer->next;
            free(adj);
        }
    }

    return answer;
}

int main() {
    size_t n, m;

    scanf("%ld %ld", &n, &m);

    Graph graph = (Graph)malloc(sizeof(Adj) * n);

    for (size_t i = 0; i < n; i++) graph[i] = NULL;    

    for (size_t i = 0; i < m; i++) {
        size_t A, B;
        scanf("%ld %ld", &A, &B);

        A--;
        B--;

        Adj adj = (Adj)malloc(sizeof(_Adj));
        adj->vertexId = B;
        adj->next = graph[A];

        graph[A] = adj;
    }

    Adj sorted = sort(n, graph);
    
    if (sorted == NULL) {
        printf("-1\n");
    } else {
        Adj it = sorted;
        while (it->next != NULL) {
            printf("%ld %ld\n", it->vertexId + 1, it->next->vertexId + 1);

            Adj adj = it;
            it = it->next;
            free(adj);
        }

        free(it);
    }

    for (size_t i = 0; i < n; i++) {
        while (graph[i] != NULL) {
            Adj adj = graph[i];
            graph[i] = adj->next;
            free(adj);
        }
    }

    free(graph);

    return 0;
}
