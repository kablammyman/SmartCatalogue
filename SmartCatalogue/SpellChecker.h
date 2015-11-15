#pragma once

#include <string>
#include <vector>

using namespace std;

class SpellChecker
{
	
private:
	vector<string> dictionary;  
	size_t levenshtein_distance(string first, string second);
public:
	SpellChecker();
	SpellChecker(vector<string> &dict);

	void addWordToDictionary(string word);
	vector<string> nearest_words(const string& word);
	

};