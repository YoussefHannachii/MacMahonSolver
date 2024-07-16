#include <iostream>
#include <fstream>
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <string>
#include <functional>
#include <stack>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>
 
using namespace std;

int gridDimension;
vector<string> carresDuJeu;
stack<string> carresDuJeuUtilise;
bool solutionTrouve;
mutex mtx;
condition_variable cv;


void recuperationDataDuFichierTxt(){
    ifstream fichier("./dim4.txt", ios::in);  // on ouvre le fichier en lecture
 
        if(fichier)  // si l'ouverture a réussi
        {       
            // instructions
                string ligne;
                char dimension;
                int cpt=0;
                fichier.get(dimension);
                gridDimension=dimension-'0';
                while(getline(fichier, ligne))  
                {
                        if(cpt!=0){
                            ligne.erase(remove(ligne.begin(), ligne.end(), ' '), ligne.end());
                            carresDuJeu.emplace_back(ligne) ;
                        }
                        cpt++;  // on l'affiche
                }
                cpt=0;
                fichier.close();  // on ferme le fichier
        }
        else  // sinon
                cerr << "Impossible d'ouvrir le fichier !" << endl;
 
}
enum coin {
    HG,
    HD,
    BG,
    BD,
    NONCOIN
    //others...
};

enum bordure {
    GAUCHE,
    HAUT,
    DROITE,
    BAS,
    NONBORDURE
    //others...
};

vector<string> melangerVectorCarreDuJeu(vector<string> & carresDuJeu){
    vector<string> copieVectCarreDuJeu=carresDuJeu;
    random_device rd;
    mt19937 gen(rd());
    shuffle(copieVectCarreDuJeu.begin(), copieVectCarreDuJeu.end(), gen);
    return copieVectCarreDuJeu;
}


void affichageDesHautDuCarre(string carre,int j){    
    if(j==0)
        cout<<"   ";
    cout<<carre[1]<<"   |   ";
}    

void affichageDesCotesDuCarre(string carre){
    cout<<carre[0]<<"     "<<carre[2]<<"|";
}    

void affichageDesBasDuCarre(string carre,int j){
    if(j==0)
        cout<<"   ";
    cout<<carre[3]<<"   |   ";
}  

void affichageDuPlateau( vector<vector<string>>& plateau,int gridDimension){
    cout<<"plateau de "<<gridDimension<<"x"<<gridDimension<<" :"<<endl;
    for (int i = 0; i < gridDimension; ++i) {
        for (int j = 0; j < gridDimension; ++j) {
            affichageDesHautDuCarre(plateau[i][j],j);
        }
       cout<<endl;
       for(int k=0; k < gridDimension; k++){
            affichageDesCotesDuCarre(plateau[i][k]);
       }
       cout<<endl;
       
       for (int p = 0; p < gridDimension; ++p) {
            affichageDesBasDuCarre(plateau[i][p],p);
        }
        cout<<endl;
        cout<<endl;
    }    
}

char couleurBordure(vector<vector<string>>& plateau){
    return plateau[0][0][0];
}

bordure carreExterne ( int row, int col){
    if(row==0){
        return HAUT;
    }
    else if(col==0){
        return GAUCHE;
    }
    else if(col==gridDimension-1){
        return DROITE;
    }
    else if(row==gridDimension-1){
        return BAS;
    }
    else{
        return NONBORDURE;
    }
}

coin carreCoin (int row, int col){
    if(row==0&&col==0){
        return HG;
    }
    else if(row==0&&col==gridDimension-1){
        return HD;
    }
    else if(row==gridDimension-1&&col==0){
        return BG;
    }
    else if(row==gridDimension-1&&col==gridDimension-1){
        return BD;
    }
    else{
        return NONCOIN;
    }
}
bool valideGaucheAdj(string carrePossible,string carreEnGauche){
    if((carrePossible[0]==carreEnGauche[2])||(carreEnGauche=="0"))
        return true;
    else
        return false;
}
bool valideHautAdj(string carrePossible,string carreEnHaut){
     if((carrePossible[1]==carreEnHaut[3])||(carreEnHaut=="0"))
        return true;
    else
        return false;
}
bool valideDroiteAdj(string carrePossible,string carreEnDroite){
    if((carrePossible[2]==carreEnDroite[0])||(carreEnDroite=="0"))
        return true;
    else
        return false;
}
bool valideBasAdj(string carrePossible,string carreEnBas){
    if((carrePossible[3]==carreEnBas[1])||(carreEnBas=="0"))
        return true;
    else
        return false;
}


bool estValideCouleurAdj(vector<vector<string>>& plateau,int row, int col, string carrePossible){
     if(carreExterne(row,col)!=NONBORDURE){
        //Cas où carré coin
        if(carreCoin(row,col)!=NONCOIN){
            coin coinIndice = carreCoin(row,col);
            switch(coinIndice){
                case 0:
                if((valideDroiteAdj(carrePossible,plateau[row][col+1]))&&
                (valideBasAdj(carrePossible,plateau[row+1][col]))){

                  return true;
                }
                else{
                    return false;
                }
                case 1:
                
                if((valideGaucheAdj(carrePossible,plateau[row][col-1]))&&
                (valideBasAdj(carrePossible,plateau[row+1][col]))){
                      return true;
                }
                else{
                    return false;
                }
                case 2:
                if((valideDroiteAdj(carrePossible,plateau[row][col+1]))&&
                (valideHautAdj(carrePossible,plateau[row-1][col]))){
                      return true;
                }
                else{
                    return false;
                }
                case 3:
                if((valideHautAdj(carrePossible,plateau[row-1][col]))&&
                (valideGaucheAdj(carrePossible,plateau[row][col-1]))){
                      return true;
                }
                else{
                    return false;
                }
            }
        }
        bordure bordureExterneIndice = carreExterne(row,col);       
        switch(bordureExterneIndice){
            case 0:
            if((valideHautAdj(carrePossible,plateau[row-1][col]))&&
                (valideDroiteAdj(carrePossible,plateau[row][col+1]))&&
                  (valideBasAdj(carrePossible,plateau[row+1][col]))){
                    return true;
                }
                else{
                    return false;
                }
            case 1:
            if((valideGaucheAdj(carrePossible,plateau[row][col-1]))&&
                (valideDroiteAdj(carrePossible,plateau[row][col+1]))&&
                  (valideBasAdj(carrePossible,plateau[row+1][col]))){
                    return true;
                }
                else{
                    return false;
                }
            case 2:
            if((valideGaucheAdj(carrePossible,plateau[row][col-1]))&&
                (valideHautAdj(carrePossible,plateau[row-1][col]))&&
                 (valideBasAdj(carrePossible,plateau[row+1][col]))){
                    return true;
                }
                else{
                    return false;
                }
            case 3:
            if((valideGaucheAdj(carrePossible,plateau[row][col-1]))&&
                (valideHautAdj(carrePossible,plateau[row-1][col]))&&
                 (valideDroiteAdj(carrePossible,plateau[row][col+1]))){
                    return true;
                }
                else{
                    return false;
                }
        }
    }
    else{
        if(valideGaucheAdj(carrePossible,plateau[row][col-1])&&
            valideHautAdj(carrePossible,plateau[row-1][col])&&
            valideDroiteAdj(carrePossible,plateau[row][col+1])&&
            valideBasAdj(carrePossible,plateau[row+1][col])){
                return true;
            }
        else{
            return false;
        }
    }   
}

//Fonction pour vérifier que le carrée est valide et peut etre place dans une case du tableau
bool estValideCouleurBord( vector<vector<string>>& plateau,int row, int col, string carrePossible){
    
    if(carreExterne(row,col)!=NONBORDURE){
        //Cas où carré coin
        if(carreCoin(row,col)!=NONCOIN){
            coin coinIndice = carreCoin(row,col);
            switch(coinIndice){
                case 0:
                if(((couleurBordure(plateau)==carrePossible[1])&&(couleurBordure(plateau)==carrePossible[0]))||((carrePossible[1]==carrePossible[0])&&(couleurBordure(plateau)=='0'))){
                    return true;
                }
                else{
                    return false;
                }
                case 1:
                
                if(((couleurBordure(plateau)==carrePossible[1])&&(couleurBordure(plateau)==carrePossible[2]))||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
                case 2:
                if(((couleurBordure(plateau)==carrePossible[3])&&(couleurBordure(plateau)==carrePossible[0]))||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
                case 3:
                if(((couleurBordure(plateau)==carrePossible[3])&&(couleurBordure(plateau)==carrePossible[2]))||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
            }
        }
        bordure bordureExterneIndice = carreExterne(row,col);       
        switch(bordureExterneIndice){
            case 0:
            if((couleurBordure(plateau)==carrePossible[0])||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
            case 1:
            if((couleurBordure(plateau)==carrePossible[1])||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
            case 2:
            if((couleurBordure(plateau)==carrePossible[2])||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
            case 3:
            if((couleurBordure(plateau)==carrePossible[3])||couleurBordure(plateau)=='0'){
                    return true;
                }
                else{
                    return false;
                }
        }
    }
    else{
        return true;
    }   
    return true;
}

//Le backtracking se fait ici pour resoudre le jeu
bool resoudreJeu(vector<vector<string>>& plateau,vector<string> &carresDuJeu,stack<string> &carresDuJeuUtilise,int row,int col){
    if(solutionTrouve){
        return false;
    }
    
    //fin colonne on passe col suivante
    if (row == gridDimension - 1 && col == gridDimension) {
        return true;
    }
    //fin ligne on passe row suivante
    if (col == gridDimension) {
        col = 0;
        row++;
    }

    // case  déjà remplie on passe
    if (plateau[row][col] != "0") {
        return resoudreJeu(plateau, carresDuJeu,carresDuJeuUtilise,row, col + 1);
    }
    // BACKTRACKING

    for (string valeur : carresDuJeu) {
        if (estValideCouleurBord(plateau, row, col, valeur)&&estValideCouleurAdj(plateau,row,col,valeur)) {
            plateau[row][col] = valeur;
            carresDuJeuUtilise.push(valeur);
            auto it = find(carresDuJeu.begin(), carresDuJeu.end(), valeur);
            carresDuJeu.erase(it);
            //affichageDuPlateau(plateau,gridDimension);
            if (resoudreJeu(plateau, carresDuJeu, carresDuJeuUtilise,row, col + 1)) {
                return true;
            }
            //valeur ne mène pas à une solution===backtracking
            plateau[row][col] = "0";
            carresDuJeu.emplace_back(carresDuJeuUtilise.top());
            carresDuJeuUtilise.pop();
        }
    }
    return false;
}


void solutionParallele(vector<vector<string>>& plateau,vector<string> &carresDuJeu){
    
    vector<vector<string>> copiePlateau=plateau;
    vector<string> copieCarreDuJeuShfld=melangerVectorCarreDuJeu(carresDuJeu);
    stack<string> carresDuJeuUtiliseDansCetteFonction;

    if(resoudreJeu(copiePlateau,copieCarreDuJeuShfld,carresDuJeuUtiliseDansCetteFonction,0,0)){
        {unique_lock<mutex> lock(mtx);
        plateau=copiePlateau;
        solutionTrouve=true;
        affichageDuPlateau(plateau,gridDimension);}
    }

}

int main(){
        recuperationDataDuFichierTxt();
        
        //création plateau vide
        vector<vector<string>> plateau(gridDimension,vector<string>(gridDimension,"0"));
        const int nombreDeThreads = 26;
        vector<thread> vecteurDeThreads;

        
        
        //cout<<estValide(plateau,1,0,"GGBB");
        //affichageDuPlateau(plateau,gridDimension);
        
        
        //solution séquentielle
        /*if (resoudreJeu(plateau, carresDuJeu, carresDuJeuUtilise,0, 0)) {
        cout << "Solution trouvée : " << endl;
        affichageDuPlateau(plateau,gridDimension);
        } else {
        cout << "Aucune solution trouvée." << endl;
        }*/

        //solution parallele
        for (int i = 0; i < nombreDeThreads; ++i) {
            vecteurDeThreads.push_back(thread(solutionParallele,ref(plateau),ref(carresDuJeu)));
        }
        
        for(thread & th : vecteurDeThreads){
            th.join();
        }

        return 0;
}