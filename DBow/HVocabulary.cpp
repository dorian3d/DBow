/**
 * File: HVocabulary.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: hierarchical vocabulary implementing (Nister, 2006)
 */

#include "HVocabulary.h"
#include "HVocParams.h"

#include "DUtils.h"

#include <cassert>
#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>
#include <fstream>
using namespace std;

using namespace DBow;

// Use Kmeans++
// (no other method supported currently)
#define KMEANS_PLUS_PLUS

HVocabulary::HVocabulary(const HVocParams &params):
	Vocabulary(params), m_params(params)
{
	assert(params.k > 1 && params.L > 0);
}

HVocabulary::HVocabulary(const char *filename) :
	Vocabulary(HVocParams(0,0)), m_params(HVocParams(0,0))
{
	Load(filename);
}

HVocabulary::HVocabulary(const HVocabulary &voc) :
	Vocabulary(voc), m_params(voc.m_params)
{
	m_nodes = voc.m_nodes;
	
	m_words.clear();
	m_words.resize(voc.m_words.size());

	vector<Node*>::const_iterator it;
	for(it = voc.m_words.begin(); it != voc.m_words.end(); it++){
		const Node *p = *it;
		m_words[it - voc.m_words.begin()] = &m_nodes[p->Id];
	}

}

HVocabulary::~HVocabulary(void)
{
}

void HVocabulary::Create(const vector<vector<float> >& training_features)
{
	// expected_nodes = Sum_{i=0..L} ( k^i )
	int expected_nodes = 
		(int)((pow((double)m_params.k, (double)m_params.L + 1) - 1)/(m_params.k - 1));
	
	// remove previous tree, allocate memory and insert root node
	m_nodes.resize(0);
	m_nodes.reserve(expected_nodes); // prevents allocations when creating the tree
	m_nodes.push_back(Node(0)); // root

	// prepare data
	int nfeatures = 0;	
	for(unsigned int i = 0; i < training_features.size(); i++){
		assert(training_features[i].size() % m_params.DescriptorLength == 0);
		nfeatures += training_features[i].size() / m_params.DescriptorLength; 
	}

	vector<pFeature> pfeatures;
	pfeatures.reserve(nfeatures);

	vector<vector<float> >::const_iterator it;
	vector<float>::const_iterator jt;

	for(it = training_features.begin(); it != training_features.end(); it++){
		for(jt = it->begin(); jt < it->end(); jt += m_params.DescriptorLength){
			pfeatures.push_back( jt );
		}
	}

	vector<float> buffer;
	buffer.reserve( m_params.k * m_params.DescriptorLength );

	// start hierarchical kmeans
	HKMeansStep(0, pfeatures, 1, buffer);

	// create word nodes
	CreateWords();

	// set the flag
	m_created = true;

	// set node weigths
	SetNodeWeights(training_features);

}

void HVocabulary::HKMeansStep(NodeId parentId, const vector<pFeature> &pfeatures, 
							  int level, vector<float>& clusters)
{
	if(pfeatures.empty()) return;

	// features associated to each cluster
	vector<vector<unsigned int> > groups; // indices from pfeatures
	groups.reserve(m_params.k);

	// number of final clusters
	int nclusters = 0;

	if((int)pfeatures.size() <= m_params.k){
		
		// trivial case: if there is a few features, each feature is a cluster
		nclusters = pfeatures.size();
		clusters.resize(pfeatures.size() * m_params.DescriptorLength);
		groups.resize(pfeatures.size());

		for(unsigned int i = 0; i < pfeatures.size(); i++){
			copy(pfeatures[i], pfeatures[i] + m_params.DescriptorLength, 
				clusters.begin() + i * m_params.DescriptorLength);
			groups[i].push_back(i);
		}
	
	}else{ // choose clusters with kmeans++

		bool first_time = true;
		bool goon = true;
		vector<pFeature>::const_iterator fit;

		// to check if clusters move after iterations
		vector<int> last_association, current_association;

		while(goon){
			// 1. Calculate clusters
			
			if(first_time){
				// random sample 

#ifdef KMEANS_PLUS_PLUS
				RandomClustersPlusPlus(clusters, pfeatures);
#else
#error No initial clustering method
#endif			
				nclusters = clusters.size() / m_params.DescriptorLength;
				
			}else{
				// calculate cluster centres
				
				vector<float>::iterator pfirst, pend, cit;
				vector<unsigned int>::const_iterator vit;
				
				for(int i = 0; i < nclusters; i++){
					pfirst = clusters.begin() + i * m_params.DescriptorLength;
					pend = clusters.begin() + (i+1) * m_params.DescriptorLength;

					fill(pfirst, pend, 0.f);

					for(vit = groups[i].begin(); vit != groups[i].end(); vit++){
						fit = pfeatures.begin() + *vit;
						// Possible improvement: divide this into chunks of 4 operations
						for(cit = pfirst; cit != pend; cit++){
							*cit += *((*fit) + (cit - pfirst));
						}
					}

					for(cit = pfirst; cit != pend; cit++) *cit /= groups[i].size();
				}
				
			} // if(first_time)

			// 2. Associate features with clusters
			
			// calculate distances to cluster centers
			groups.clear();
			groups.resize(nclusters, vector<unsigned int>());
			current_association.resize(pfeatures.size());

			for(fit = pfeatures.begin(); fit != pfeatures.end(); fit++){
				double best_sqd = DescriptorSqDistance(*fit, clusters.begin());
				int icluster = 0;

				for(int i = 1; i < nclusters; i++){
					double sqd = DescriptorSqDistance(*fit,
						clusters.begin() + i * m_params.DescriptorLength);

					if(sqd < best_sqd){
						best_sqd = sqd;
						icluster = i;
					}
				}
				groups[icluster].push_back(fit - pfeatures.begin());
				current_association[ fit - pfeatures.begin() ] = icluster;
			}

			// remove clusters with no features
			// (this is not necessary with kmeans++)
#ifndef KMEANS_PLUS_PLUS
			for(int i = nclusters-1; i >= 0; i--){
				if(groups[i].empty()){
					groups.erase(groups.begin() + i);
					clusters.erase(clusters.begin() + i * m_params.DescriptorLength, 
						clusters.begin() + (i+1) * m_params.DescriptorLength);
				}
			}
			nclusters = groups.size();
#endif

			// 3. check convergence
			if(first_time){
				first_time = false;
			}else{
				goon = false;
				for(unsigned int i = 0; i < current_association.size(); i++){
					if(current_association[i] != last_association[i]){
						goon = true;
						break;
					}
				}
			}

			if(goon){
				// copy last feature-cluster association
				last_association = current_association;
			}

		} // while(goon)

	} // if trivial case

	// Kmeans done, create nodes
	
	// create child nodes
	for(int i = 0; i < nclusters; i++){
		NodeId id = m_nodes.size();
		m_nodes.push_back(Node(id));
		m_nodes.back().Descriptor.resize(m_params.DescriptorLength);
		copy(clusters.begin() + i * m_params.DescriptorLength, 
			clusters.begin() + (i+1) * m_params.DescriptorLength,
			m_nodes.back().Descriptor.begin());
		
		m_nodes[parentId].Children.push_back(id);
	}

	if(level < m_params.L){
		// iterate again with the resulting clusters
		for(int i = 0; i < nclusters; i++){
			NodeId id = m_nodes[m_nodes[parentId].Children[i]].Id;
			
			vector<pFeature> child_features;
			child_features.reserve(groups[i].size());

			vector<unsigned int>::const_iterator vit;
			for(vit = groups[i].begin(); vit != groups[i].end(); vit++){
				child_features.push_back(pfeatures[*vit]);
			}

			if(child_features.size() > 1){
				// (clusters variable can be safely reused now)
				HKMeansStep(id, child_features, level + 1, clusters);
			}
		}
	}
}

int HVocabulary::GetNumberOfWords() const
{
	return m_words.size(); 
}

void HVocabulary::SaveBinary(const char *filename) const
{
	// Format (binary):
	// [Header]
	// k L N
	// NodeId_1 ParentId Weight d1 ... d_D
	// ...
	// NodeId_(N-1) ParentId Weight d1 ... d_D
	// WordId_0 frequency NodeId
	// ...
	// WordId_(N-1) frequency NodeId
	//
	// Where:
	// k (int32): branching factor
	// L (int32): depth levels
	// N (int32): number of nodes, including root
	// NodeId (int32): root node is not present. Not in order
	// ParentId (int32)
	// Weight (double64)
	// d_i (float32): descriptor entry
	// WordId (int32): in ascending order
	// frequency (float32): frequency of word
	// NodeId (int32): node associated to word
	//
	// (the number along with the data type represents the size in bits)

	DUtils::BinaryFile f(filename, DUtils::WRITE);

	const int N = m_nodes.size();

	// header
	SaveBinaryHeader(f);
	f << m_params.k << m_params.L << N;

	// tree
	vector<NodeId> parents, children;
	vector<NodeId>::const_iterator pit;

	parents.push_back(0); // root

	while(!parents.empty()){
		NodeId pid = parents.back();
		parents.pop_back();

		const Node& parent = m_nodes[pid];
		children = parent.Children;

		for(pit = children.begin(); pit != children.end(); pit++){
			const Node& child = m_nodes[*pit];

			// save node data
			f << (int)child.Id << (int)pid << (double)child.Weight;
			for(int i = 0; i < m_params.DescriptorLength; i++){
				f << child.Descriptor[i];
			}

			// add to parent list
			if(!child.isLeaf()){
				parents.push_back(*pit);
			}
		}
	}

	// vocabulary
	vector<Node*>::const_iterator wit;
	for(wit = m_words.begin(); wit != m_words.end(); wit++){
		WordId id = wit - m_words.begin();
		f << (int)id << GetWordFrequency(id) << (int)(*wit)->Id;
	}

	f.Close();
}

void HVocabulary::SaveText(const char *filename) const
{
	// Format (text)
	// [Header]
	// k L N
	// NodeId_1 ParentId Weight d1 ... d_D
	// ...
	// NodeId_(N-1) ParentId Weight d1 ... d_D
	// WordId_0 frequency NodeId
	// ...
	// WordId_(N-1) frequency NodeId

	fstream f(filename, ios::out);
	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	// magic word is not necessary in the text file

	f.precision(10);

	const int N = m_nodes.size();

	// header
	SaveTextHeader(f);
	f << m_params.k << " " << m_params.L << " " << N << endl;
	
	// tree
	vector<NodeId> parents, children;
	vector<NodeId>::const_iterator pit;

	parents.push_back(0); // root

	while(!parents.empty()){
		NodeId pid = parents.back();
		parents.pop_back();

		const Node& parent = m_nodes[pid];
		children = parent.Children;

		for(pit = children.begin(); pit != children.end(); pit++){
			const Node& child = m_nodes[*pit];

			// save node data
			f << child.Id << " "
				<< pid << " "
				<< child.Weight << " ";
			for(int i = 0; i < m_params.DescriptorLength; i++){
				f << child.Descriptor[i] << " ";
			}
			f << endl;

			// add to parent list
			if(!child.isLeaf()){
				parents.push_back(*pit);
			}
		}
	}

	// vocabulary
	vector<Node*>::const_iterator wit;
	for(wit = m_words.begin(); wit != m_words.end(); wit++){
		WordId id = wit - m_words.begin();
		f << (int)id << " "
			<< GetWordFrequency(id) << " "
			<< (int)(*wit)->Id
			<< endl;
	}

	f.close();
}

unsigned int HVocabulary::LoadBinary(const char *filename)
{
	// Format (binary):
	// [Header]
	// k L N
	// NodeId_1 ParentId Weight d1 ... d_D
	// ...
	// NodeId_(N-1) ParentId Weight d1 ... d_D
	// WordId_0 frequency NodeId
	// ...
	// WordId_(N-1) frequency NodeId

	DUtils::BinaryFile f(filename, DUtils::READ);

	int nwords = LoadBinaryHeader(f);

	_load<DUtils::BinaryFile>(f, nwords);

	unsigned int ret = f.BytesRead();

	f.Close();

	return ret;
}

unsigned int HVocabulary::LoadText(const char *filename)
{
	// Format (text)
	// [Header]
	// k L N
	// NodeId_1 ParentId Weight d1 ... d_D
	// ...
	// NodeId_(N-1) ParentId Weight d1 ... d_D
	// WordId_0 frequency NodeId
	// ...
	// WordId_(N-1) frequency NodeId

	fstream f(filename, ios::in);
	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	int nwords = LoadTextHeader(f);

	_load<fstream>(f, nwords);

	unsigned int ret = (unsigned int)f.tellg();

	f.close();

	return ret;
}


template<class T>
void HVocabulary::_load(T &f, int nwords)
{
	// general header has already been read,
	// giving value to these member variables
	int nfreq = m_frequent_words_stopped;
	int ninfreq = m_infrequent_words_stopped;

	// removes nodes, words and frequencies
	m_created = false;
	m_words.clear();
	m_nodes.clear();
	m_word_frequency.clear();

	// h header
	int nnodes;
	f >> m_params.k >> m_params.L >> nnodes;

	// creates all the nodes at a time
	m_nodes.resize(nnodes);
	m_nodes[0].Id = 0; // root node

	for(int i = 1; i < nnodes; i++){
		int nodeid, parentid;
		double weight;
		f >> nodeid >> parentid >> weight;

		m_nodes[nodeid].Id = nodeid;
		m_nodes[nodeid].Weight = weight;
		m_nodes[parentid].Children.push_back(nodeid);

		m_nodes[nodeid].Descriptor.resize(m_params.DescriptorLength);
		for(int j = 0; j < m_params.DescriptorLength; j++){
			f >> m_nodes[nodeid].Descriptor[j];
		}
	}

	m_words.resize(nwords);
	m_word_frequency.resize(nwords);

	for(int i = 0; i < nwords; i++){
		int wordid, nodeid;
		float frequency;
		f >> wordid >> frequency >> nodeid;
		
		m_nodes[nodeid].WId = wordid;
		m_words[wordid] = &m_nodes[nodeid];
		m_word_frequency[wordid] = frequency;
	}

	// all was ok
	m_created = true;

	// create an empty stop list
	CreateStopList();

	// and stop words
	StopWords(nfreq, ninfreq);
}


void HVocabulary::RandomClustersPlusPlus(vector<float>& clusters, 
										 const vector<pFeature> &pfeatures) const
{
	// Implements kmeans++ seeding algorithm
	// Algorithm:
	// 1. Choose one center uniformly at random from among the data points.
	// 2. For each data point x, compute D(x), the distance between x and the nearest 
	//    center that has already been chosen.
	// 3. Add one new data point as a center. Each point x is chosen with probability 
	//    proportional to D(x)^2.
	// 4. Repeat Steps 2 and 3 until k centers have been chosen.
	// 5. Now that the initial centers have been chosen, proceed using standard k-means 
	//    clustering.

	clusters.resize(m_params.k * m_params.DescriptorLength);

	vector<bool> feature_used(pfeatures.size(), false);

	// 1.
	int ifeature = DUtils::Random::RandomInt(0, pfeatures.size()-1);
	feature_used[ifeature] = true;
	
	// create first cluster
	copy(pfeatures[ifeature], pfeatures[ifeature] + m_params.DescriptorLength, 
		clusters.begin());
	int used_clusters = 1;
	

	vector<double> sqdistances; 
	vector<int> ifeatures;

	sqdistances.reserve(pfeatures.size());
	ifeatures.reserve(pfeatures.size());

	vector<pFeature>::const_iterator fit;

	while(used_clusters < m_params.k){
		// 2.
		sqdistances.resize(0);
		ifeatures.resize(0);
		
		for(fit = pfeatures.begin(); fit != pfeatures.end(); fit++){
			ifeature = fit - pfeatures.begin();
			if(!feature_used[ifeature]){
				
				double min_sqd = DescriptorSqDistance(*fit, clusters.begin());
				for(int i = 1; i < used_clusters; i++){
					double sqd = DescriptorSqDistance(*fit, 
						clusters.begin() + i * m_params.DescriptorLength);

					if(sqd < min_sqd){
						min_sqd = sqd;
					}
				}

				sqdistances.push_back(min_sqd);
				ifeatures.push_back(ifeature);
			}
		}

		// 3.
		double sqd_sum = accumulate(sqdistances.begin(), sqdistances.end(), 0.0);

		if(sqd_sum > 0){
			double cut_d;
			do{
				cut_d = DUtils::Random::RandomValue<double>(0, sqd_sum);
			}while(cut_d == 0.0);

			double d_up_now = 0;
			vector<double>::iterator dit;
			for(dit = sqdistances.begin(); dit != sqdistances.end(); dit++){
				d_up_now += *dit;
				if(d_up_now >= cut_d) break;
			}
			if(dit == sqdistances.end()) dit = sqdistances.begin() + sqdistances.size()-1;

			ifeature = ifeatures[dit - sqdistances.begin()];

			assert(!feature_used[ifeature]);

			copy(pfeatures[ifeature], pfeatures[ifeature] + m_params.DescriptorLength, 
				clusters.begin() + used_clusters * m_params.DescriptorLength);
			feature_used[ifeature] = true;
			used_clusters++;
			
		}else
			break;
	}

	if(used_clusters < m_params.k)
		clusters.resize(used_clusters * m_params.DescriptorLength);

}


double HVocabulary::DescriptorSqDistance(const pFeature &v, 
			const pFeature &w) const
{
	double sqd = 0.0;

	const int rest = m_params.DescriptorLength % 4;

	for(int i = 0; i < m_params.DescriptorLength - rest; i += 4){
		sqd += (*(v + i) - *(w + i)) * (*(v + i) - *(w + i));
		sqd += (*(v + i + 1) - *(w + i + 1)) * (*(v + i + 1) - *(w + i + 1));
		sqd += (*(v + i + 2) - *(w + i + 2)) * (*(v + i + 2) - *(w + i + 2));
		sqd += (*(v + i + 3) - *(w + i + 3)) * (*(v + i + 3) - *(w + i + 3));
	}

	for(int i = m_params.DescriptorLength - rest; i < m_params.DescriptorLength; i++){
		sqd += (*(v + i) - *(w + i)) * (*(v + i) - *(w + i));
	}

	return sqd;
}

void HVocabulary::SetNodeWeights(const vector<vector<float> >& training_features)
{
	vector<WordValue> weights;
	GetWordWeightsAndCreateStopList(training_features, weights);

	assert(weights.size() == m_words.size());

	for(unsigned int i = 0; i < m_words.size(); i++){
		m_words[i]->Weight = weights[i];
	}
}

WordId HVocabulary::Transform(const vector<float>::const_iterator &pfeature) const
{
	if(isEmpty()) return 0;

	assert(!m_nodes[0].isLeaf());

	// propagate the feature down the tree
	vector<NodeId> nodes;
	vector<NodeId>::const_iterator it;
	
	NodeId final_id = 0; // root

	do{
		nodes = m_nodes[final_id].Children;
		final_id = nodes[0];
		double best_sqd = DescriptorSqDistance(pfeature, m_nodes[final_id].Descriptor.begin());

		for(it = nodes.begin() + 1; it != nodes.end(); it++){
			NodeId id = *it;
			double sqd = DescriptorSqDistance(pfeature, m_nodes[id].Descriptor.begin());
			if(sqd < best_sqd){
				best_sqd = sqd;
				final_id = id;
			}
		}
	} while( !m_nodes[final_id].isLeaf() );
	
	// turn node id into word id
	return m_nodes[final_id].WId;
}

void HVocabulary::CreateWords()
{
	m_words.resize(0);
	m_words.reserve( (int)pow((double)m_params.k, (double)m_params.L) );

	// the actual order of the words is not important
	vector<Node>::iterator it;
	for(it = m_nodes.begin(); it != m_nodes.end(); it++){
		if(it->isLeaf()){
			it->WId = m_words.size();
			m_words.push_back( &(*it) );
		}
	}
}

WordValue HVocabulary::GetWordWeight(WordId id) const
{
	if(isEmpty()) return 0;

	assert(id < m_words.size());

	return m_words[id]->Weight;
}
