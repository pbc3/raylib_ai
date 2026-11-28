

Question | Implementation

1.2 Player Implementation - [player.cpp](player.cpp), [input.cpp](input.cpp)
1.3 - 1.5:
Two Distinct Enemy Types[fsm.cpp](fsm.cpp)
Chaser Enemy FSM -  [fsm.cpp](fsm.cpp) - inside update_enemy_A
Guard Enemy FSM - [fsm.cpp](fsm.cpp) - inside update_enemy_B
The chaser enemy uses Astar pathfinding, while the guard enemy uses Dijkstra - [pathfinding.cpp](pathfinding.cpp)

1.4 - the enemies collide with walls and closed doors, and take damage when hit by the player. 

1.6 Agent state visualisation - [renderer.cpp](renderer.cpp) inside draw_enemies() and draw_enemy_info()


2.1 Purposeful terrain design & navigation challenges.

2.2 Procedural artefact generation - [artefacts.cpp](artefacts.cpp). mainly inside generate_artefact_locations(). minimum spacing to each artefact is maintained, and aminimum distance from the players spawn.
2.3 NavMesh accessibility verification or technically equivalent approach - I used astar to ensure a path from the player exists to the location.

Advanced Implementation
Option A - probalistic damage modelling. i implemented a symmetric 2d6 damage modifier centered around the base damage of 20, with an additional 10 percent critical hit chance. - [player.cpp](player.cpp)
Option C - I added 3 terrain types, normal ground, water and a fast tile. fast ground carries a weight of 1, normal ground 2, and water 3. This influences path finding for the enemies, as they opt to go on the fast tile where possible.