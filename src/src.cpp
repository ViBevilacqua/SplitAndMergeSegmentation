//Sviluppato da ViBevilacqua
//Se si sceglie una soglia bassa, le regioni piu' difficilmente si uniranno, e di conseguenza ci sara' non solo un numero elevato di regioni, ma si noteranno complessivamente anche piu' dettagli nell'immagine
//Se si sceglie una soglia alta, le regioni si uniranno piu' facilmente poiche' il predicato di omogenieta' risultera' vero molte volte, di conseguenza alla fine si avranno meno regioni, e nell'immagine complessiva si vedranno solo le forme in grandi linee con molti pochi dettagli(anche senza).

//Statuetta appoggiata sul muro: soglia1 = 140, soglia2 = 30

#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

void split(Mat img, Mat regione, int& id, int soglia);

void colorazione(Mat img, Mat& output, Mat regione);
void merge(Mat img, Mat regione, int soglia);

vector <vector <Vec3b>> colori;
vector <Vec3b> centroidi;
int main(int argc, char * argv[])
{
    if(argc < 4)
    {
        cout  << "Errore, inserire: <./eseguibile> <path_img> <Soglia_split> <Soglia_Merge>" << endl;
        exit (-1);
    }

    Mat img = imread(argv[1], IMREAD_COLOR);
    Mat regione(img.rows, img.cols, CV_32SC1, Scalar(0));
    int id = -1;

    namedWindow("input", WINDOW_NORMAL);
    blur(img, img, Size(3,3));
    imshow("input", img);

    split(img, regione, id, atoi(argv[2]));
    string nome = argv[1];
    Mat output(img.rows, img.cols, CV_8UC3);
    colorazione(img, output, regione);
    namedWindow("Fase di Split [con bordi]", WINDOW_NORMAL);
    imshow("Fase di Split [con bordi]", output);
    imwrite("split(con_bordi)_"+nome, output);
    merge(output, regione, atoi(argv[3]));

    colorazione(output, output, regione);

    namedWindow("Fase di Merge [con bordi]", WINDOW_NORMAL);
    imshow("Fase di Merge [con bordi]", output);
    imwrite("merge(con_bordi)_"+nome, output);
    waitKey(0);
    return 0;
}

void split(Mat img, Mat regione, int& id, int soglia)
{
    Scalar med, devStd;
    med = mean(img);
    Vec3b centroide(cvRound(med[0]), cvRound(med[1]), cvRound(med[2]));
    vector<double> distanza;
    for (int i = 0; i < img.rows; i++)
        for(int j = 0; j < img.cols; j++)
            distanza.push_back(norm(img.at<Vec3b>(i,j), centroide));

    meanStdDev(distanza, Scalar(), devStd);

    if((devStd[0] * devStd[0]) < soglia || img.rows <= 8)
    {
        id ++;
        vector <Vec3b> coloriReg;
        for(int i = 0;  i < img.rows; i++)
            for(int j = 0; j < img.cols; j++)
            {
                regione.at<int>(i,j) = id;
                coloriReg.push_back(img.at<Vec3b>(i,j));
            }
        colori.push_back(coloriReg);
        centroidi.push_back(centroide);
        return;
    }

    int dim = img.rows/2;

    split(img(Rect(0, 0, dim, dim)), regione(Rect(0, 0, dim, dim)), id, soglia);
    split(img(Rect(dim, 0, dim, dim)), regione(Rect(dim, 0, dim, dim)), id, soglia);
    split(img(Rect(0, dim, dim, dim)), regione(Rect(0, dim, dim, dim)), id, soglia);
    split(img(Rect(dim, dim, dim, dim)), regione(Rect(dim, dim, dim, dim)), id, soglia);

};


void merge(Mat img, Mat regione, int soglia)
{

    int r[] = {0, 1};
    int c[] = {1, 0};
    bool unione;

    do
    {
        unione = false;
        vector <vector <int>> adiacenzeControllate;
        adiacenzeControllate.resize(centroidi.size());
        for(int i = 0; i < img.rows-1; i++)
            for(int j = 0; j < img.cols-1; j++)
                for(int k = 0; k < 2; k++)
                    if(regione.at<int>(i,j) != regione.at<int>(i+r[k],j+c[k]))
                    {
                        int regA = regione.at<int>(i,j);
                        int regB = regione.at<int>(i+r[k],j+c[k]);
                        bool nuovo = true;
                        for(int w = 0; w < adiacenzeControllate[regA].size(); w++)
                            if(adiacenzeControllate[regA][w] == regB)
                            {
                                nuovo = false;
                                break;
                            }
                        if(nuovo)
                            if(norm(centroidi[regA], centroidi[regB]) <= soglia)
                            {
                                vector<Vec3b> regioneTemp(colori[regA]);
                                regioneTemp.insert(regioneTemp.end(), colori[regB].begin(), colori[regB].begin());
                                Scalar med = mean(regioneTemp);
                                centroidi[regA] = Vec3b(cvRound(med[0]), cvRound(med[1]), cvRound(med[2]));
                                for(int w = 0; w < img.rows; w++)
                                    for(int z = 0; z < img.cols; z++)
                                        if(regione.at<int>(w,z) == regB)
                                            regione.at<int>(w,z) = regA;
                                colori.at(regA) = regioneTemp;
                                unione = true;
                            }
                            else
                            {
                                adiacenzeControllate[regA].push_back(regB);
                                adiacenzeControllate[regB].push_back(regA);
                            }
                    }
    }while(unione);
}


void colorazione(Mat img, Mat& output, Mat regione)
{
    for(int i = 0; i < img.rows; i++)
        for(int j = 0; j < img.cols; j++)
            output.at<Vec3b>(i,j) = centroidi[regione.at<int>(i,j)];

    copyMakeBorder(regione, regione, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(-1));

   for(int i=1; i  < regione.rows-1;i++)
    for(int j=1; j < regione.cols-1; j++)
        if(regione.at<int>(i,j) != regione.at<int>(i-1,j) || regione.at<int>(i,j) != regione.at<int>(i,j+1) || regione.at<int>(i,j)!= regione.at<int>(i+1,j) || regione.at<int>(i,j)!= regione.at<int>(i,j-1))
            output.at<Vec3b>(i-1, j-1) = Vec3b(0, 0, 0);
    regione = regione(Rect(1,1, img.cols, img.rows));
}
