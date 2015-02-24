#include<stdio.h>
#include <string.h>

// functie ce imi bordeaza matricea in forma de toroid pentru a calcula suma vecinilor
// functia primeste ca parametri dimensiunile matricii, matricea si noua matrice bordata
void reprToroid(int row, int col, int harta[row][col], int toroid[row + 2][col + 2]){
	int i, j;
	#pragma omp parallel for private(i, j) collapse(2)
	for(i = 1; i <= row; i++){
		for(j = 1; j <= col; j++){
			toroid[i][j] = harta[i - 1][j - 1];
			toroid[0][j] =  harta[row - 1][j - 1];
			toroid[row + 1][j] = harta[0][j - 1];
		}
	}
	#pragma omp parallel for private(i)
	for(i = 0; i <=row + 1; i++){
		toroid[i][0] = toroid[i][col];
		toroid[i][col + 1] = toroid[i][1];
	} 
}

// functie ce imi calculeaza suma vecinilor unui individ
// primeste ca argumente matricea si dimensiunile ei si pozitia individului caruia
// vreau sa-i calculez suma vecinilor
int sumaVecini(int row, int col, int matrix[row][col], int pozRow, int pozCol){
	int suma = 0;
	int k, i, j;
	suma = matrix[pozRow - 1][pozCol - 1] + matrix[pozRow - 1][pozCol]+ 
	matrix[pozRow - 1][pozCol + 1] + matrix[pozRow][pozCol - 1]+
	matrix[pozRow][pozCol + 1] + matrix[pozRow + 1][pozCol - 1]
	+ matrix[pozRow + 1][pozCol] + matrix[pozRow + 1][pozCol + 1];
    		
	return suma;
}

//functie ce imi bordeaza matricea cu 0 pentru a calcula suma vecinilor
// functia primeste ca parametri dimensiunile matricii, matricea si noua matrice bordata
void bordare(int row, int col, int matrix[row][col], int borderedMatrix[row + 2][col + 2]){
	int i, j;
	// pastrez vechea matrice
        for(i = 1; i <= row; i++){
                for(j = 1; j <= col; j++)
                        borderedMatrix[i][j] = matrix[i - 1][j - 1];
                borderedMatrix[0][i] =  0;
                borderedMatrix[row + 1][i] = 0;
        }
        // adaug zerouri pe margine
        for(i = 0; i <=col+1; i++){
		borderedMatrix[i][0] = 0;
                borderedMatrix[i][col + 1] = 0;
        }
}

//functie ce scrie o matrice intr-un fisier
void writeFile(int row, int col, int matrix[row][col], char form, char* output_file){
	FILE *f = fopen(output_file, "w");
    	int i, j, maxi = 0, maxj = 0, nri = 0, nrj = 0;
    	fprintf(f, "%c ", form);
    	// caut pozitia liniei si coloanei maxime de afisare
    	for(i = row-1; i >= 0; i--){
        	for(j = col-1; j >= 0; j--){
			if(matrix[i][j] == 1){
        	        	maxi = i;
        	            	nri++;
            		}
            	if(nri == 1)
                	goto label1;
        	}
    	}
    	label1:
    	for(j = col-1; j >= 0; j--){
        	for(i = row-1; i >= 0; i--){
        		if(matrix[i][j] == 1){
        		        maxj = j;
        		        nrj++;
            		}
            		if(nrj == 1)
                		goto label;
        	}
    	}
	label: 
	fprintf(f, "%d %d %d %d\n", maxj + 1, maxi + 1, col, row);
	for(i = 0; i <= maxi; i++){
                for(j = 0; j <= maxj; j++)
                        fprintf(f, "%d ", matrix[i][j]);
                fprintf(f, "\n");
        }
	fclose(f);
}
int main(int argc, char* argv[]){
	int num_threads = atoi(argv[1]), num_steps = atoi(argv[2]);
	char litera, input_file[25], output_file[25];
	strcpy(input_file, argv[3]);
	strcpy(output_file, argv[4]);
	int W_harta, H_harta, W, H, i, j, n, k, nr;
	FILE *fr = fopen(input_file,"r");
	// citesc din fisier forma hartii, dimensiunile si matricea
	fscanf(fr,"%c %d %d %d %d", &litera, &W_harta, &H_harta, &W, &H);
	//setez numarul de threaduri
	omp_set_num_threads(num_threads);
	int sume[H][W], simulare[H][W], simulareBordata[H + 2][W + 2];
	for(i = 0; i < H; i++)
		for(j = 0; j < W; j++)
			simulare[i][j] = 0;
	for(i = 0; i < H_harta; i++){
		for(j = 0; j < W_harta; j++){
			fscanf(fr, "%d", &nr);
			if( i < H && j < W) 
				simulare[i][j] = nr;
		}
	}
	if(litera == 'P')
		bordare(H, W, simulare, simulareBordata);
	else
		reprToroid(H, W, simulare, simulareBordata);
	for(k = 0; k < num_steps; k++){
		// generez matricea de sume de vecini
		#pragma omp parallel for private(i, j) collapse(2)
		for(i = 1; i <= H; i++){
	        	for(j = 1; j <= W; j++)
	                	sume[i - 1][j - 1] = sumaVecini(H + 2, W + 2, simulareBordata, i, j);
	        }
	    	// generez starea urmatoare in functie de reguli
	    	#pragma omp parallel for private(i, j) collapse(2)
	    	for(i = 0; i < H; i++){
			for(j = 0; j < W; j++){
				if((sume[i][j] <= 1) || (sume[i][j] > 3))
					simulare[i][j] = 0;
				else if((sume[i][j] == 3) && (simulare[i][j] == 0)) 
					simulare[i][j] = 1;
			}
		}
		// actualizez si matricea bordata
		if(litera == 'P'){
			#pragma omp parallel for private(i, j) collapse(2)
			for(i = 0; i < H; i++){
				for(j = 0; j < W; j++)
					simulareBordata[i + 1][j + 1] = simulare[i][j];
			}
			
		}
		else 
			reprToroid(H, W, simulare, simulareBordata);
	}
	writeFile(H, W, simulare, litera, output_file);
	fclose(fr);
	return 0;
}
