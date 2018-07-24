# 🗺️ mapgen - A small map generator demo.

_mapgen 🗺️_'s objective is to build a simple map generator using simple rules and applying them on a cell-grid. The "simple rules" are inspired by computer vision, among others.

### mapgen 🗺️
_mapgen 🗺️_ currently generates a map by randomly filling the grid with walls and then erodes the walls to destroy
The grid can be seen as a 4-connected graph.
  - ZQSD to move around
  - Space to zoom 10x on the map (space again to unzoom)

### mapplay 🎮
_mapplay 🎮_ allows you to explore the map. You are the red dot.
  - ZQSD to move around
  - Space to destroy 4 walls around you

### Dependencies
- C++11
- SFML 2.5
