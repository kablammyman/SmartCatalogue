#include "stdafx.h"

#include "SpellChecker.h"

#include <numeric>
#include <algorithm>

SpellChecker::SpellChecker()
{

}

SpellChecker::SpellChecker(vector<string> &dict)
{
	dictionary = dict;
}

void SpellChecker::addWordToDictionary(string word)
{
	//dont add a word we already have
	for (size_t i = 0; i < dictionary.size(); i++)
		if (dictionary[i] == word)
			return;

	dictionary.push_back(word);
}
//----------------------------------------------------------------------
size_t SpellChecker::levenshtein_distance(string first, string second)
{
	if (second.size() < first.size()) swap(first, second);
	vector<size_t> current(second.size() + 1), previous(current);
	iota(previous.begin(), previous.end(), 0);

	for (size_t i = 0; i < first.size(); ++i)
	{
		current[0] = i + 1;
		for (size_t j = 0; j < second.size(); ++j)
			current[j + 1] = min(min(current[j], previous[j + 1]) + 1, previous[j] + (first[i] != second[j]));

		current.swap(previous);
	}
	return previous.back();
}
//----------------------------------------------------------------------
vector<string> SpellChecker::nearest_words(const string& word)
{
	constexpr size_t MAX_DISTANCE = 5;
	vector<string> nearest[MAX_DISTANCE + 1];
	for (const string match : dictionary)
	{
		size_t dist = levenshtein_distance(word, match);
		if (dist <= MAX_DISTANCE)
			nearest[dist].push_back(match);
	}

	for (const auto& seq : nearest)
		if (!seq.empty())
			return move(seq);
	return{};
}
//----------------------------------------------------------------------
/*void SpellChecker::testSpellchecker()
{
	const std::vector<std::string>& dict{ "mess", "less", "plus", "pious", "messes", "misses", "lessen", "listen" };
	const std::string test[] = { "mises", "missse", "mesten", "pius" };
	for (const std::string& word : test)
	{
		std::cout << word << " => ";
		for (const auto& nearest : nearest_words(word, dict))
			std::cout << nearest << ' ';
		std::cout << '\n';
	}
}*/