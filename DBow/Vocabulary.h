/**
 * File: Vocabulary.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: generic vocabulary that must be inherited
 */

#pragma once
#ifndef __VOCABULARY__
#define __VOCABULARY__

#include "VocParams.h"
#include "VocInfo.h"
#include "BowVector.h"
#include "DUtils.h"

namespace DBow {

	class Database;

	class Vocabulary
	{
	public:

		/**
		 * Creates an empty vocabulary with the given parameters
		 * @param params vocbulary parameters
		 */
		Vocabulary(const VocParams &params);

		/**
		 * Copy constructor. Allocates new data
		 * @param voc vocabulary to copy
		 */
		Vocabulary(const Vocabulary &voc);

		/**
		 * Loads a stored vocabulary
		 */
		Vocabulary(const char *filename);

		/** Destructor
		 */
		virtual ~Vocabulary(void);

		/**
		 * Saves the current vocabulary in filename.
		 * The vocabulary can be saved in binary format (improves size and speed)
		 * or in text format (good for interoperability).
		 * Note that training data is not saved.
		 * @param filename file to store the vocabulary in
		 * @param binary (default: true): sets if binary format must be used
		 */
		void Save(const char *filename, bool binary = true) const;

		/**
		 * Loads a stored vocabulary (except for its training data).
		 * The current vocabulary is cleared.
		 * @param filename file to load
		 * @return number of bytes or chars read
		 */
		unsigned int Load(const char *filename);

		/**
		 * Returns whether the vocabulary is empty or has already been created
		 */
		inline bool isEmpty() const { return !m_created; }

		/** 
		 * Creates the vocabulary from some training data. 
		 * The current content of the vocabulary is cleared
		 * @param training_features vector of groups of features in the OpenCV format.
		 *    Each feature group represents a different source of features.
		 *    This is necessary for some weighting methods, like tf-idf
		 */
		virtual void Create(const vector<vector<float> >& training_features) = 0;

		/** 
		 * Transforms a set of image features into a bag-of-words vector
		 * according to the current vocabulary.
		 * Stopped words must not be returned
		 * @see Vocabulary::isWordStopped
		 * @param features image features in the OpenCV format
		 * @param v (out) bow vector
		 * @param arrange (default: true) iif true, puts entries in v in order.
		 *    This is necessary if v is going to be used with Vocabulary::Score.
		 *    If not (is only used with Database), setting arrange to false can
		 *    slightly save some time
		 */
		void Transform(const vector<float>& features, BowVector &v, bool arrange = true) const;

		/** 
		 * Returns the number of words in the vocabulary
		 * @return number of words
		 */
		inline int NumberOfWords() const
		{
			if(m_created) return GetNumberOfWords();
			else return 0;
		}

		/**
		 * Stops frequent or infrequent words from the vocabulary.
		 * When a word is stopped, bow vectors returned by later ::Transform calls
		 * will not contain it. BowVectors created before stopping words may still
		 * contain stopped words, so that scores calculated from them will not
		 * take stopped words into account. Therefore, for correct scoring, 
		 * bow vectors must be calculated after stopping words.
		 * Calls to ::StopWords do not stack, so that words can be considered
		 * again by calling ::StopWords with lower frequencies.
		 * Particularly, StopWords(0,0) disables the stop list.
		 *
		 * @param frequent_words how many frequent words must be stopped, 
		 *    in per one units 
		 * @param infrequent_words (default 0) how many infrequent words must be stopped,
		 *    in per one units
		 */
		void StopWords(float frequent_words, float infrequent_words = 0);

		/** 
		 * Stops frequent or infrequent words from the vocabulary.
		 * @see ::StopWords(float, float)
		 * @param frequent_words how many frequent words must be stopped
		 * @param infrequent_words how many infrequent words must be stopped
		 */
		void StopWords(int frequent_words, int infrequent_words = 0);
		
		/** 
		 * Retrieves all the information about the vocabulary
		 * @return information
		 */
		VocInfo RetrieveInfo() const;

		/** 
		 * Gets the weighting method
		 * @return weighting method
		 */
		inline VocParams::WeightingType Weighting() const {
			return m_params->Weighting;
		}

		/** 
		 * Gets the scoring method
		 * @return scoring method
		 */
		inline VocParams::ScoringType Scoring() const { 
			return m_params->Scoring;
		}

		/** 
		 * Returns the score between two vectors according to this voc.
		 * BowVectors must be in order of ids
		 * @param v the first bow vector
		 * @param w the second one
		 * @return score
		 */
		double Score(const BowVector &v, const BowVector &w) const;

	protected:

		/** 
		 * Saves the vocabulary in binary format. Must be implemented 
		 * by subclasses.
		 * @param filename file to store the vocabulary in
		 */
		virtual void SaveBinary(const char *filename) const = 0;

		/** 
		 * Saves the vocabulary in text format. Must be implemented 
		 * by subclasses.
		 * @param filename file to store the vocabulary in
		 */
		virtual void SaveText(const char *filename) const = 0;

		/** 
		 * Loads the vocabulary in binary format. Must be implemented 
		 * by subclasses.
		 * @param filename file to read the vocabulary from 
		 * @return number of bytes read
		 */
		virtual unsigned int LoadBinary(const char *filename) = 0;

		/** 
		 * Loads the vocabulary in text format. Must be implemented 
		 * by subclasses.
		 * @param filename file to read the vocabulary from 
		 * @return number of chars read 
		 */
		virtual unsigned int LoadText(const char *filename) = 0;

		/**
		 * Transforms a feature into its word id
		 * @param feature descriptor. Pointer to the beginning of a DescriptorLenght
		 *     size vector containing the feature descriptor
		 * @return word id
		 */
		virtual WordId Transform(const vector<float>::const_iterator &pfeature) const = 0;

		/**
		 * Returns the weight of a word
		 * @param id word id
		 * @return word weight
		 */
		virtual WordValue GetWordWeight(WordId id) const = 0;

		/**
		 * Returns the number of words in the vocabulary
		 * (must not check m_created)
		 * @return number of words
		 */
		virtual int GetNumberOfWords() const = 0;

		/**
		 * Returns the frequency of a word
		 * @param id word id
		 * @return word frequency
		 */
		float GetWordFrequency(WordId id) const;

		/**
		 * Checks if a given word is stopped
		 * @return true iif the word is stopped
		 */
		inline bool isWordStopped(WordId id) const {
			return(id < m_word_stopped.size() && m_word_stopped[id]);
		}

		/**
		 * Saves a header with vocabulary info in binary format
		 * @param f (in/out) file
		 */
		void SaveBinaryHeader(DUtils::BinaryFile &f) const;

		/**
		 * Saves a header with vocabulary info in binary text
		 * @param f (in/out) file
		 */
		void SaveTextHeader(fstream &f) const;

		/**
		 * Loads header with vocabulary info in binary format
		 * @param f (in/out) file
		 * @return number of words
		 */
		int LoadBinaryHeader(DUtils::BinaryFile &f);

		/**
		 * Loads a header with vocabulary info in binary format
		 * @param f (in/out) file
		 * @return number of words
		 */
		int LoadTextHeader(fstream &f);

	protected:

		/**
		 * Calculates the weights of all the words in the vocabulary.
		 * This function must be called in ::Create after creating the vocabulary
		 * @param training_features features used to create the vocabulary
		 *    (same format as in ::Create)
		 * @param weigths (out) vector such that weights[WordId] = weight
		 */
		void GetWordWeightsAndCreateStopList(const vector<vector<float> >& training_features,
			vector<WordValue> &weights);

		/**
		 * Creates an empty stop list with the word frequencies given.
		 * m_word_frequency must be filled for all the words in the vocabulary.
		 * This function is called by GetWordWeightsAndCreateStopList, but
		 * can be also called by subclasses if they have filled m_word_frequency.
		 */
		void CreateStopList();

	protected:

		// Says if the vocabulary was already created
		// Must be flagged by subclasses
		bool m_created;

		// Number of words stopped
		int m_frequent_words_stopped;
		int m_infrequent_words_stopped;

		// Stores the frequency of each word: m_word_frequency[word_id] = fr
		vector<float> m_word_frequency;

	private:

		// Vocabulary parameters
		VocParams *m_params;

		// Allows quick access to check if a word is stopped
		// m_word_stopped[word_id] = true/false
		vector<bool> m_word_stopped;

		// Stores ids of currently stopped words
		vector<WordId> m_stop_list;

		// Stores the ids of the words in order of ascending frequency
		vector<WordId> m_words_in_order;

	};

}

#endif

