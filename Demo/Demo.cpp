
#include <iostream>
#include <vector>

// OpenCV
#include <cv.h>
#include <highgui.h>

// DBow
#include "DUtils.h"
#include "DBow.h"

using namespace DBow;
using namespace DUtils;
using namespace std;

void loadFeatures(vector<vector<float> > &features);
void testVocCreation(const vector<vector<float> > &features);
void testDatabase(const vector<vector<float> > &features);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// number of training images
const int Nimages = 4;

// extended surf gives 128-dimensional vectors
const bool extended_surf = false;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void wait()
{
	cout << endl << "Press enter to continue" << endl;
	getchar();
}

int main()
{
	vector<vector<float> > features;
	loadFeatures(features);

	testVocCreation(features);

	wait();

	testDatabase(features);

	wait();

	return 0;
}

void loadFeatures(vector<vector<float> > &features)
{
	features.clear();
	features.reserve(Nimages);
	
	cv::SURF surf(400, 4, 2, extended_surf);

	cout << "Extracting SURF features..." << endl;
	for(int i = 1; i <= Nimages; i++){
		stringstream ss;
		ss << "image" << i << ".png";

		cv::Mat image = cv::imread(ss.str(), 0);
		cv::Mat mask;
		vector<cv::KeyPoint> keypoints;
		
		features.push_back(vector<float>());
		surf(image, mask, keypoints, features.back());
	}
}

void testVocCreation(const vector<vector<float> > &features)
{
	// branching factor and depth levels 
	const int k = 9;
	const int L = 3;

	HVocParams params(k, L, (extended_surf ? 128 : 64));
	HVocabulary voc(params);

	cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
	voc.Create(features);
	cout << "... done!" << endl;

	cout << "Stopping some words..." << endl;
	voc.StopWords(0.01f);

	cout << "Vocabulary information: " << endl;
	cout << endl << voc.RetrieveInfo().toString() << endl;

	// lets do something with this vocabulary
	cout << "Matching images against themselves (0 low, 1 high): " << endl;
	BowVector v1, v2;
	for(int i = 0; i < Nimages; i++){
		voc.Transform(features[i], v1);
		for(int j = i+1; j < Nimages; j++){
			voc.Transform(features[j], v2);

			double score = voc.Score(v1, v2);
			cout << "Image " << i+1 << " vs Image " << j+1 << ": " << score << endl;
		}
	}

	cout << endl << "Saving vocabulary..." << endl;
	voc.Save("small_vocabulary.txt", false);
	cout << "Done" << endl;

}

void testDatabase(const vector<vector<float> > &features)
{
	cout << "Creating a small database..." << endl;

	HVocabulary *voc = new HVocabulary("small_vocabulary.txt");
	Database db(*voc);
	delete voc; // db maintains its own vocabulary instance
	
	for(int i = 0; i < Nimages; i++){
		db.AddEntry(features[i]);
	}

	cout << "... done!" << endl;

	cout << "Database information: " << endl;
	cout << endl << db.RetrieveInfo().toString() << endl;

	cout << "Querying the database: " << endl;
	
	QueryResults ret;
	for(int i = 0; i < Nimages; i++){
		db.Query(ret, features[i], 2);
		
		// ret[0] is always the same image in this case, because we added it to the 
		// database. ret[1] is the second best match.

		cout << "Searching for Image " << i+1 << ". Best match: " 
			<< ret[1].Id + 1 << ", score: " << ret[1].Score << endl;
	}

}
