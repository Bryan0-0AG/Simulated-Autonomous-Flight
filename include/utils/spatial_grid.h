#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include <vector>
#include <cmath>
#include "utils/vector2.h"
#include "body.h"

class SpatialGrid {
public:
    SpatialGrid(float width, float height, float cellSize) 
        : WIDTH(width), HEIGHT(height), CELL_SIZE(cellSize) {
        COLS = static_cast<int>(std::ceil(width / cellSize));
        ROWS = static_cast<int>(std::ceil(height / cellSize));
        CELLS.resize(COLS * ROWS);
    }

    void clear() {
        for (auto& cell : CELLS) {
            cell.clear();
        }
    }

    void addBody(Body* body) {
        int col = static_cast<int>(body->position.x / CELL_SIZE);
        int row = static_cast<int>(body->position.y / CELL_SIZE);

        if (col >= 0 && col < COLS &&
            row >= 0 && row < ROWS) {
            CELLS[row * COLS + col].push_back(body);
        }
    }

    std::vector<Body*> getNeighbors(const Body& body) {
        std::vector<Body*> neighbors;
        int actual_col = static_cast<int>(body.position.x / CELL_SIZE);
        int actual_row = static_cast<int>(body.position.y / CELL_SIZE);

        /*      
           -1                  0                 1
        left_col      <    actual_col      >   right_col
        bottom_row    <    actual_row      >   top_row 
        */        
        for (int dc = -1; dc <= 1; ++dc) {      
            for (int dr = -1; dr <= 1; ++dr) {  
                int neighbor_col = actual_col + dc;
                int neighbor_row = actual_row + dr;

                if (neighbor_col >= 0 && neighbor_col < COLS &&
                    neighbor_row >= 0 && neighbor_row < ROWS) {
                        
                    const auto& cell = CELLS[neighbor_row * COLS + neighbor_col];
                    neighbors.insert(neighbors.end(), cell.begin(), cell.end());
                }
            }
        }
        return neighbors;
    }

    Vector2 getCellByPos(Vector2 position) {
        int col = static_cast<int>(position.x / CELL_SIZE);
        int row = static_cast<int>(position.y / CELL_SIZE);
        
        // El centro es (Columna * Tamaño + Mitad del Tamaño)
        return {
            (col * CELL_SIZE) + (CELL_SIZE / 2.0f),
            (row * CELL_SIZE) + (CELL_SIZE / 2.0f)
        };
    }

    Vector2 getCellByCoord(int col, int row) {
        return {
            (col * CELL_SIZE) + (CELL_SIZE / 2.0f),
            (row * CELL_SIZE) + (CELL_SIZE / 2.0f)
        };
    }

private:
    float WIDTH, HEIGHT, CELL_SIZE;
    int COLS, ROWS;
    std::vector<std::vector<Body*>> CELLS;
};

#endif
