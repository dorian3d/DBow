/**
 * File: HVocabulary.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: hierarchical vocabulary implementing (Nister, 2006)
 */

#pragma once
#ifndef __H_VOCABULARY__
#define __H_VOCABULARY__

#include "Vocabulary.h"
#include "BowVector.h"
#include "HVocParams.h"

#include <vector>
using namespace std;

namespace DBow {

	class HVocabulary :
		public Vocabulary
	{
	public:

		/**
		 * Constructor
		 * @param params vocabulary parameters
		 */
		HVocabulary(const HVocParams &params);

		/**
		 * Copy constructor. Allocates new data
		 * @param voc vocabulary to copy
		 */
		HVocabulary(const HVocabulary &voc);

		/**
		 * Constructor
		 * @param filename file to load in
		 */
		HVocabulary(const char *filename);

		/** 
		 * Destructor
		 */
		~HVocabulary(void);

		/** 
		 * Creates the vocabulary from some training data. 
		 * The current content of the vocabulary is cleared
		 * @see Vocabulary::Create
		 * @param training_features vector of groups of features in the OpenCV format.
		 */
		void Create(const vector<vector<float> >& training_features);

		/**
		 * Transforms a set of features into a bag-of-words vector
		 * @see Vocabulary::Transform
		 */
		using Vocabulary::Transform;

	protected:
		
		/** 
		 * Saves the vocabulary in binary format.
		 * @param filename file to store the vocabulary in
		 */
		void SaveBinary(const char *filename) const;

		/** 
		 * Saves the vocabulary in text format.
		 * @param filename file to store the vocabulary in
		 */
		void SaveText(const char *filename) const;

		/** 
		 * Loads the vocabulary in binary format.
		 * @param filename file to read the vocabulary from 
		 */
		unsigned int LoadBinary(const char *filename);

		/** 
		 * Loads the vocabulary in text format. 
		 * @param filename file to read the vocabulary from 
		 */
		unsigned int LoadText(const char *filename);

		/**
		 * Returns the weight of a word
		 * @see Vocabulary::GetWordWeight
		 * @param id word id
		 * @return word weight
		 */
		WordValue GetWordWeight(WordId id) const;

		/**
		 * Returns the number of words in the vocabulary
		 * (must not check m_created)
		 * @return number of words
		 */
		int GetNumberOfWords() const;

		/**
		 * Transforms a feature into its word id
		 * @see Vocabulary::Transform
		 * @param feature descriptor. Pointer to the beginning of a DescriptorLenght
		 *     size vector containing the feature descriptor
		 * @return word id
		 */
		WordId Transform(const vector<float>::const_iterator &pfeature) const;

	protected:

		// Voc parameters
		HVocParams m_params;

		typedef unsigned int NodeId;
		typedef unsigned int DocId;

		struct Node {
			NodeId Id;
			vector<NodeId> Children;
			WordValue Weight;
			vector<float> Descriptor;
			
			WordId WId; // if this node is a leaf, it will have a word id

			/**
			 * Constructor
			 */
			Node(): Id(0), Weight(0), WId(-1){}
			Node(NodeId _id): Id(_id), Weight(0), WId(-1){}

			/**
			 * Returns if the node is a leaf node
			 * @return true iif the node is a leaf
			 */
			inline bool isLeaf() const { return Children.empty(); }
		};

		// Nodes in the tree, including root [0] with no descriptor
		vector<Node> m_nodes;

		// The words of the vocabulary are the tree leaves
		vector<Node*> m_words;

		// Pointer to a feature (only used when Creating the vocabulary)
		typedef vector<float>::const_iterator pFeature;

	protected:

		/**
		 * Performs kmeans recursively and created the vocabulary tree.
		 * Nodes are created without weights
		 * @param parentId created nodes will be children of parentId
		 * @param pfeatures data to perform the kmeans
		 * @param level current tree level (starting in 1)
		 * @param clusters a buffer to reuse in all the HKMeansStep calls.
		 *    It should be a vector with memory allocated for k * DescriptorLength
		 *    floats
		 */
		void HKMeansStep(NodeId parentId, const vector<pFeature> &pfeatures, 
			int level,  vector<float>& clusters);

		/**
		 * Initiates clusters by using the algorithm of kmeans++
		 * @param clusters (out) clusters created. Its size is multiple of
		 *    DescriptoLength
		 * @param pfeatures features in the data space to create the clusters
		 */
		void RandomClustersPlusPlus(vector<float>& clusters, 
									const vector<pFeature> &pfeatures) const;
		
		/**
		 * Calculates the Euclidean squared distance between two features
		 * @param v
		 * @param w
		 * @return squared distance
		 */
		double DescriptorSqDistance(const pFeature &v, const pFeature &w) const;

		/**
		 * Sets the node weights once the tree has been created according to
		 * the data used
		 * @param training_features features used to create the vocabulary
		 */
		void SetNodeWeights(const vector<vector<float> >& training_features);

		/**
		 * Creates the words of the vocabulary once the tree is built
		 */
		void CreateWords();

	private:

		/**
		 * Loads data from a file stream.
		 * The generic header must already be read
		 * @param f file stream opened in reading mode
		 * @param nwords number of words in the voc
		 */
		template<class T> void _load(T &f, int nwords);

	};

}

#endif

