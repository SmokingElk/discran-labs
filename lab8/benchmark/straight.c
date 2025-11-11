#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef struct Adj {
    size_t vertexId;
    struct Adj* next;
} _Adj, *Adj;

typedef Adj* Graph;

// Переборный алгоритм топологической сортировки
Adj bruteForceSort(size_t n, Graph graph) {
    // Массив для отслеживания посещенных вершин в текущем пути
    bool* visited = (bool*)malloc(sizeof(bool) * n);
    // Массив для отслеживания вершин в текущем пути (для обнаружения циклов)
    bool* inPath = (bool*)malloc(sizeof(bool) * n);
    // Массив для отслеживания уже добавленных в ответ вершин
    bool* added = (bool*)malloc(sizeof(bool) * n);
    // Массив для хранения полустепеней захода вершин
    size_t* inDegree = (size_t*)malloc(sizeof(size_t) * n);
    
    // Инициализация массивов
    for (size_t i = 0; i < n; i++) {
        visited[i] = false;
        inPath[i] = false;
        added[i] = false;
        inDegree[i] = 0;
    }
    
    // Вычисление полустепеней захода
    for (size_t i = 0; i < n; i++) {
        for (Adj it = graph[i]; it != NULL; it = it->next) {
            inDegree[it->vertexId]++;
        }
    }
    
    Adj answer = NULL;
    size_t addedCount = 0;
    bool changed = true;
    
    // Основной цикл - на каждой итерации ищем вершины с нулевой полустепенью захода
    while (changed && addedCount < n) {
        changed = false;
        
        // Перебираем все вершины
        for (size_t i = 0; i < n; i++) {
            // Если вершина еще не добавлена и у нее нулевая полустепень захода
            if (!added[i] && inDegree[i] == 0) {
                changed = true;
                added[i] = true;
                addedCount++;
                
                // Добавляем вершину в начало списка ответа
                Adj newAdj = (Adj)malloc(sizeof(_Adj));
                newAdj->vertexId = i;
                newAdj->next = answer;
                answer = newAdj;
                
                // Уменьшаем полустепени захода для всех смежных вершин
                for (Adj it = graph[i]; it != NULL; it = it->next) {
                    inDegree[it->vertexId]--;
                }
            }
        }
    }
    
    // Освобождение памяти
    free(visited);
    free(inPath);
    free(added);
    free(inDegree);
    
    // Если не все вершины были добавлены - значит есть цикл
    if (addedCount != n) {
        while (answer != NULL) {
            Adj temp = answer;
            answer = answer->next;
            free(temp);
        }
        return NULL;
    }
    
    return answer;
}

// Функция для разворота списка (чтобы порядок был корректным)
Adj reverseList(Adj head) {
    Adj prev = NULL;
    Adj current = head;
    Adj next = NULL;
    
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    
    return prev;
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

    clock_t start = clock(); 
    // Используем переборный алгоритм вместо DFS
    Adj sorted = bruteForceSort(n, graph);
    
    // Разворачиваем список, так как алгоритм Кана дает обратный порядок
    sorted = reverseList(sorted);

    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);
    
    if (sorted == NULL) {
        printf("-1\n");
    } else {
        Adj it = sorted;
        while (it != NULL) {
            printf("%ld ", it->vertexId + 1);
            Adj adj = it;
            it = it->next;
            free(adj);
        }
        printf("\n");
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