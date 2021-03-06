local ns = {}
setmetatable(ns, {__index = _G})
desert_cave_walls = ns;
setfenv(1, ns);

file_name = "dat/tilesets/desert_cave_walls.lua"
image = "img/tilesets/desert_cave_walls.png"
num_tile_cols = 16
num_tile_rows = 16

walkability = { [0] = {}, [1] = {}, [2] = {}, [3] = {}, 
		[4]= {}, [5] = {}, [6] = {}, [7] = {},
		[8] = {}, [9] = {}, [10] = {}, [11] = {}, 
		[12] = {}, [13] = {}, [14] = {}, [15] = {} 
}


walkability[0][0]   = { 0, 0, 0, 0 }
walkability[0][1]   = { 0, 0, 0, 0 }
walkability[0][2]   = { 0, 0, 0, 0 }
walkability[0][3]   = { 0, 0, 0, 0 }
walkability[0][4]   = { 0, 0, 0, 0 }
walkability[0][5]   = { 1, 1, 1, 1 }
walkability[0][6]   = { 1, 1, 1, 1 }
walkability[0][7]   = { 1, 1, 1, 1 }
walkability[0][8]   = { 1, 1, 1, 1 }
walkability[0][9]   = { 1, 1, 1, 1 }
walkability[0][10]  = { 1, 1, 1, 1 }
walkability[0][11]  = { 1, 1, 1, 1 }
walkability[0][12]  = { 1, 1, 1, 1 }
walkability[0][13]  = { 1, 1, 1, 1 }
walkability[0][14]  = { 1, 0, 1, 0 }
walkability[0][15]  = { 0, 0, 0, 0 }
walkability[1][0]   = { 0, 0, 0, 0 }
walkability[1][1]   = { 0, 0, 0, 0 }
walkability[1][2]   = { 0, 0, 0, 0 }
walkability[1][3]   = { 0, 0, 0, 0 }
walkability[1][4]   = { 1, 1, 1, 1 }
walkability[1][5]   = { 1, 1, 1, 1 }
walkability[1][6]   = { 1, 1, 1, 1 }
walkability[1][7]   = { 1, 1, 1, 1 }
walkability[1][8]   = { 1, 1, 1, 1 }
walkability[1][9]   = { 1, 1, 1, 1 }
walkability[1][10]  = { 1, 1, 1, 1 }
walkability[1][11]  = { 1, 1, 1, 1 }
walkability[1][12]  = { 1, 1, 1, 1 }
walkability[1][13]  = { 1, 1, 1, 1 }
walkability[1][14]  = { 1, 1, 1, 1 }
walkability[1][15]  = { 0, 0, 1, 1 }
walkability[2][0]   = { 0, 0, 0, 0 }
walkability[2][1]   = { 1, 1, 1, 1 }
walkability[2][2]   = { 1, 1, 1, 1 }
walkability[2][3]   = { 0, 0, 0, 0 }
walkability[2][4]   = { 1, 1, 1, 1 }
walkability[2][5]   = { 1, 1, 1, 1 }
walkability[2][6]   = { 1, 1, 1, 1 }
walkability[2][7]   = { 1, 1, 1, 1 }
walkability[2][8]   = { 1, 1, 1, 1 }
walkability[2][9]   = { 1, 1, 1, 1 }
walkability[2][10]  = { 1, 1, 1, 1 }
walkability[2][11]  = { 1, 1, 1, 1 }
walkability[2][12]  = { 1, 1, 1, 1 }
walkability[2][13]  = { 1, 1, 1, 1 }
walkability[2][14]  = { 0, 0, 0, 0 }
walkability[2][15]  = { 0, 1, 0, 1 }
walkability[3][0]   = { 0, 0, 0, 1 }
walkability[3][1]   = { 1, 1, 1, 1 }
walkability[3][2]   = { 1, 1, 1, 1 }
walkability[3][3]   = { 0, 0, 1, 0 }
walkability[3][4]   = { 1, 1, 1, 1 }
walkability[3][5]   = { 1, 1, 1, 1 }
walkability[3][6]   = { 1, 1, 0, 0 }
walkability[3][7]   = { 1, 1, 0, 0 }
walkability[3][8]   = { 1, 1, 0, 0 }
walkability[3][9]   = { 1, 1, 0, 0 }
walkability[3][10]  = { 1, 1, 0, 0 }
walkability[3][11]  = { 1, 1, 0, 0 }
walkability[3][12]  = { 1, 1, 1, 1 }
walkability[3][13]  = { 1, 1, 1, 1 }
walkability[3][14]  = { 0, 0, 1, 1 }
walkability[3][15]  = { 1, 1, 1, 1 }
walkability[4][0]   = { 0, 1, 0, 1 }
walkability[4][1]   = { 1, 1, 1, 1 }
walkability[4][2]   = { 1, 1, 1, 1 }
walkability[4][3]   = { 1, 0, 1, 0 }
walkability[4][4]   = { 1, 1, 1, 1 }
walkability[4][5]   = { 1, 1, 1, 1 }
walkability[4][6]   = { 1, 1, 1, 1 }
walkability[4][7]   = { 1, 1, 1, 1 }
walkability[4][8]   = { 1, 1, 1, 1 }
walkability[4][9]   = { 1, 1, 1, 1 }
walkability[4][10]  = { 0, 0, 0, 0 }
walkability[4][11]  = { 0, 0, 0, 0 }
walkability[4][12]  = { 0, 0, 0, 0 }
walkability[4][13]  = { 0, 0, 0, 0 }
walkability[4][14]  = { 0, 0, 0, 0 }
walkability[4][15]  = { 0, 0, 0, 0 }
walkability[5][0]   = { 1, 1, 1, 1 }
walkability[5][1]   = { 1, 1, 1, 1 }
walkability[5][2]   = { 1, 1, 1, 1 }
walkability[5][3]   = { 1, 1, 1, 1 }
walkability[5][4]   = { 1, 1, 1, 1 }
walkability[5][5]   = { 1, 1, 1, 1 }
walkability[5][6]   = { 1, 1, 1, 1 }
walkability[5][7]   = { 1, 1, 1, 1 }
walkability[5][8]   = { 1, 1, 1, 1 }
walkability[5][9]   = { 1, 1, 1, 1 }
walkability[5][10]  = { 0, 0, 0, 0 }
walkability[5][11]  = { 0, 0, 0, 0 }
walkability[5][12]  = { 0, 0, 1, 1 }
walkability[5][13]  = { 0, 0, 1, 1 }
walkability[5][14]  = { 0, 0, 1, 1 }
walkability[5][15]  = { 0, 0, 1, 1 }
walkability[6][0]   = { 1, 1, 1, 1 }
walkability[6][1]   = { 1, 1, 1, 1 }
walkability[6][2]   = { 1, 1, 1, 1 }
walkability[6][3]   = { 1, 1, 1, 1 }
walkability[6][4]   = { 1, 1, 1, 1 }
walkability[6][5]   = { 1, 1, 1, 1 }
walkability[6][6]   = { 1, 1, 1, 1 }
walkability[6][7]   = { 1, 1, 1, 1 }
walkability[6][8]   = { 1, 1, 1, 1 }
walkability[6][9]   = { 1, 1, 1, 1 }
walkability[6][10]  = { 0, 0, 0, 0 }
walkability[6][11]  = { 0, 0, 0, 0 }
walkability[6][12]  = { 0, 0, 0, 0 }
walkability[6][13]  = { 0, 0, 0, 0 }
walkability[6][14]  = { 0, 0, 0, 0 }
walkability[6][15]  = { 0, 0, 0, 0 }
walkability[7][0]   = { 0, 0, 0, 0 }
walkability[7][1]   = { 0, 0, 0, 0 }
walkability[7][2]   = { 0, 0, 0, 0 }
walkability[7][3]   = { 0, 0, 0, 0 }
walkability[7][4]   = { 1, 1, 1, 1 }
walkability[7][5]   = { 1, 1, 1, 1 }
walkability[7][6]   = { 1, 1, 1, 1 }
walkability[7][7]   = { 1, 1, 1, 1 }
walkability[7][8]   = { 1, 1, 1, 1 }
walkability[7][9]   = { 1, 1, 1, 1 }
walkability[7][10]  = { 0, 0, 0, 0 }
walkability[7][11]  = { 0, 0, 0, 0 }
walkability[7][12]  = { 0, 0, 1, 1 }
walkability[7][13]  = { 0, 0, 1, 1 }
walkability[7][14]  = { 0, 0, 1, 1 }
walkability[7][15]  = { 0, 0, 1, 1 }
walkability[8][0]   = { 0, 1, 0, 1 }
walkability[8][1]   = { 1, 1, 1, 1 }
walkability[8][2]   = { 1, 1, 1, 1 }
walkability[8][3]   = { 1, 0, 1, 0 }
walkability[8][4]   = { 1, 1, 1, 1 }
walkability[8][5]   = { 1, 1, 1, 1 }
walkability[8][6]   = { 1, 1, 1, 1 }
walkability[8][7]   = { 1, 1, 1, 1 }
walkability[8][8]   = { 1, 1, 1, 1 }
walkability[8][9]   = { 1, 1, 1, 1 }
walkability[8][10]  = { 0, 0, 0, 0 }
walkability[8][11]  = { 0, 0, 0, 0 }
walkability[8][12]  = { 0, 0, 0, 0 }
walkability[8][13]  = { 0, 0, 0, 0 }
walkability[8][14]  = { 0, 0, 0, 0 }
walkability[8][15]  = { 0, 0, 0, 0 }
walkability[9][0]   = { 0, 1, 0, 1 }
walkability[9][1]   = { 1, 1, 1, 1 }
walkability[9][2]   = { 1, 1, 1, 1 }
walkability[9][3]   = { 1, 0, 1, 0 }
walkability[9][4]   = { 1, 1, 0, 0 }
walkability[9][5]   = { 1, 1, 0, 0 }
walkability[9][6]   = { 1, 1, 0, 0 }
walkability[9][7]   = { 1, 1, 0, 0 }
walkability[9][8]   = { 1, 1, 0, 0 }
walkability[9][9]   = { 1, 1, 0, 0 }
walkability[9][10]  = { 0, 0, 0, 0 }
walkability[9][11]  = { 0, 0, 0, 0 }
walkability[9][12]  = { 0, 0, 1, 1 }
walkability[9][13]  = { 0, 0, 1, 1 }
walkability[9][14]  = { 0, 0, 1, 1 }
walkability[9][15]  = { 0, 0, 1, 1 }
walkability[10][0]  = { 0, 1, 0, 1 }
walkability[10][1]  = { 1, 1, 1, 1 }
walkability[10][2]  = { 1, 1, 1, 1 }
walkability[10][3]  = { 1, 0, 1, 0 }
walkability[10][4]  = { 1, 1, 1, 1 }
walkability[10][5]  = { 1, 1, 1, 1 }
walkability[10][6]  = { 1, 1, 1, 1 }
walkability[10][7]  = { 1, 1, 1, 1 }
walkability[10][8]  = { 0, 0, 0, 0 }
walkability[10][9]  = { 0, 0, 0, 0 }
walkability[10][10] = { 1, 1, 1, 1 }
walkability[10][11] = { 1, 1, 1, 1 }
walkability[10][12] = { 1, 1, 1, 1 }
walkability[10][13] = { 1, 1, 1, 1 }
walkability[10][14] = { 1, 1, 1, 1 }
walkability[10][15] = { 1, 1, 1, 1 }
walkability[11][0]  = { 0, 1, 0, 1 }
walkability[11][1]  = { 1, 1, 1, 1 }
walkability[11][2]  = { 1, 1, 1, 1 }
walkability[11][3]  = { 1, 0, 1, 0 }
walkability[11][4]  = { 1, 1, 1, 1 }
walkability[11][5]  = { 1, 1, 1, 1 }
walkability[11][6]  = { 1, 1, 1, 1 }
walkability[11][7]  = { 1, 1, 1, 1 }
walkability[11][8]  = { 0, 0, 0, 0 }
walkability[11][9]  = { 0, 0, 0, 0 }
walkability[11][10] = { 1, 1, 1, 1 }
walkability[11][11] = { 1, 1, 1, 1 }
walkability[11][12] = { 1, 1, 1, 1 }
walkability[11][13] = { 1, 1, 1, 1 }
walkability[11][14] = { 1, 1, 1, 1 }
walkability[11][15] = { 1, 1, 1, 1 }
walkability[12][0]  = { 1, 1, 1, 1 }
walkability[12][1]  = { 1, 1, 1, 1 }
walkability[12][2]  = { 1, 1, 1, 1 }
walkability[12][3]  = { 1, 1, 1, 1 }
walkability[12][4]  = { 1, 1, 1, 1 }
walkability[12][5]  = { 1, 1, 1, 1 }
walkability[12][6]  = { 1, 1, 1, 1 }
walkability[12][7]  = { 1, 1, 1, 1 }
walkability[12][8]  = { 1, 1, 1, 1 }
walkability[12][9]  = { 1, 1, 1, 1 }
walkability[12][10] = { 1, 1, 1, 1 }
walkability[12][11] = { 1, 1, 1, 1 }
walkability[12][12] = { 1, 1, 1, 1 }
walkability[12][13] = { 1, 1, 1, 1 }
walkability[12][14] = { 1, 1, 1, 1 }
walkability[12][15] = { 1, 1, 1, 1 }
walkability[13][0]  = { 1, 1, 1, 1 }
walkability[13][1]  = { 1, 1, 1, 1 }
walkability[13][2]  = { 1, 1, 1, 1 }
walkability[13][3]  = { 1, 1, 1, 1 }
walkability[13][4]  = { 1, 1, 1, 1 }
walkability[13][5]  = { 1, 1, 1, 1 }
walkability[13][6]  = { 1, 1, 1, 1 }
walkability[13][7]  = { 1, 1, 1, 1 }
walkability[13][8]  = { 1, 1, 1, 1 }
walkability[13][9]  = { 1, 1, 1, 1 }
walkability[13][10] = { 1, 1, 1, 1 }
walkability[13][11] = { 1, 1, 1, 1 }
walkability[13][12] = { 1, 1, 1, 1 }
walkability[13][13] = { 1, 1, 1, 1 }
walkability[13][14] = { 1, 1, 1, 1 }
walkability[13][15] = { 1, 1, 1, 1 }
walkability[14][0]  = { 1, 1, 1, 1 }
walkability[14][1]  = { 1, 1, 1, 1 }
walkability[14][2]  = { 1, 1, 1, 1 }
walkability[14][3]  = { 1, 1, 1, 1 }
walkability[14][4]  = { 1, 1, 1, 1 }
walkability[14][5]  = { 1, 1, 1, 1 }
walkability[14][6]  = { 1, 1, 1, 1 }
walkability[14][7]  = { 1, 1, 1, 1 }
walkability[14][8]  = { 1, 1, 1, 1 }
walkability[14][9]  = { 1, 1, 1, 1 }
walkability[14][10] = { 1, 1, 1, 1 }
walkability[14][11] = { 1, 1, 1, 1 }
walkability[14][12] = { 1, 1, 1, 1 }
walkability[14][13] = { 1, 1, 1, 1 }
walkability[14][14] = { 1, 1, 1, 1 }
walkability[14][15] = { 1, 1, 1, 1 }
walkability[15][0]  = { 1, 1, 1, 1 }
walkability[15][1]  = { 1, 1, 1, 1 }
walkability[15][2]  = { 1, 1, 1, 1 }
walkability[15][3]  = { 1, 1, 1, 1 }
walkability[15][4]  = { 1, 1, 1, 1 }
walkability[15][5]  = { 1, 1, 1, 1 }
walkability[15][6]  = { 1, 1, 1, 1 }
walkability[15][7]  = { 1, 1, 1, 1 }
walkability[15][8]  = { 1, 1, 1, 1 }
walkability[15][9]  = { 1, 1, 1, 1 }
walkability[15][10] = { 1, 1, 1, 1 }
walkability[15][11] = { 1, 1, 1, 1 }
walkability[15][12] = { 1, 1, 1, 1 }
walkability[15][13] = { 1, 1, 1, 1 }
walkability[15][14] = { 1, 1, 1, 1 }
walkability[15][15] = { 1, 1, 1, 1 }


animated_tiles = {}
animated_tiles[1] = {170, 250, 172, 10, 174, 10, 192, 10, 194, 10, 196, 10, 198, 10, 200, 10, 202, 10, 204, 10, 206, 10, 224, 10, 226, 10, 228, 10, 230, 10, 232, 10, 234, 10, 236, 10, 238, 10, 170, 250, 238, 10, 236, 10, 234, 10, 232, 10, 230, 10, 228, 10, 226, 10, 224, 10, 206, 10, 204, 10, 202, 10, 200, 10, 198, 10, 196, 10, 194, 10, 192, 10, 174, 10}
animated_tiles[2] = {171, 250, 173, 10, 175, 10, 193, 10, 195, 10, 197, 10, 199, 10, 201, 10, 203, 10, 205, 10, 207, 10, 225, 10, 227, 10, 229, 10, 231, 10, 233, 10, 235, 10, 237, 10, 239, 10, 171, 250, 239, 10, 237, 10, 235, 10, 233, 10, 231, 10, 229, 10, 227, 10, 225, 10, 207, 10, 205, 10, 203, 10, 201, 10, 199, 10, 197, 10, 195, 10, 193, 10, 175, 10}
animated_tiles[3] = {186, 250, 188, 10, 190, 10, 208, 10, 210, 10, 212, 10, 214, 10, 216, 10, 218, 10, 220, 10, 222, 10, 240, 10, 242, 10, 244, 10, 246, 10, 248, 10, 250, 10, 252, 10, 254, 10, 186, 250, 254, 10, 252, 10, 250, 10, 248, 10, 246, 10, 244, 10, 242, 10, 240, 10, 222, 10, 220, 10, 218, 10, 216, 10, 214, 10, 212, 10, 210, 10, 208, 10, 190, 10}
animated_tiles[4] = {187, 250, 189, 10, 191, 10, 209, 10, 211, 10, 213, 10, 215, 10, 217, 10, 219, 10, 221, 10, 223, 10, 241, 10, 243, 10, 245, 10, 247, 10, 249, 10, 251, 10, 253, 10, 255, 10, 187, 250, 255, 10, 253, 10, 251, 10, 249, 10, 247, 10, 245, 10, 243, 10, 241, 10, 223, 10, 221, 10, 219, 10, 217, 10, 215, 10, 213, 10, 211, 10, 209, 10, 191, 10}
