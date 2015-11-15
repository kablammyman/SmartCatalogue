#include "stdafx.h"
#include "Trie.h"

Node* Node::findChild(char c)
{
    for ( size_t i = 0; i < mChildren.size(); i++ )
    {
        Node* tmp = mChildren.at(i);
        if ( tmp->content() == c )
        {
            return tmp;
        }
    }

    return NULL;
}

Trie::Trie()
{
    root = new Node();
}

Trie::~Trie()
{
    // Free memory
	 for ( size_t i = 0; i < root->children().size(); i++ )
		 delete root->children()[i];
	 delete root;
}

void Trie::addWord(string s)
{
    Node* current = root;

    if ( s.length() == 0 )
    {
        current->setWordMarker(); // an empty word
        return;
    }

    for ( size_t i = 0; i < s.length(); i++ )
    {        
        Node* child = current->findChild(s[i]);
        if ( child != NULL )
        {
            current = child;
        }
        else
        {
            Node* tmp = new Node();
            tmp->setContent(s[i]);
            current->appendChild(tmp);
            current = tmp;
        }
        if ( i == s.length() - 1 )
            current->setWordMarker();
    }
}


bool Trie::searchWord(string s)
{
    Node* current = root;

    while ( current != NULL )
    {
        for ( size_t i = 0; i < s.length(); i++ )
        {
            Node* tmp = current->findChild(s[i]);
            if ( tmp == NULL )
                return false;
            current = tmp;
        }

        if ( current->wordMarker() )
            return true;
        else
            return false;
    }

    return false;
}
/*

// Test program
int mainTest()
{
    Trie* trie = new Trie();
    trie->addWord("Hello");
    trie->addWord("Balloon");
    trie->addWord("Ball");

    if ( trie->searchWord("Hell") )
        cout << "Found Hell" << endl;

    if ( trie->searchWord("Hello") )
        cout << "Found Hello" << endl;

    if ( trie->searchWord("Helloo") )
        cout << "Found Helloo" << endl;

    if ( trie->searchWord("Ball") )
        cout << "Found Ball" << endl;

    if ( trie->searchWord("Balloon") )
        cout << "Found Balloon" << endl;

    delete trie;
}*/
/*
OUTPUT:-
Found Hello
Found Ball
Found Balloon
*/