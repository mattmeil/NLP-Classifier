#include <iostream>
#include "csvstream.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <set>
using namespace std;
class Classifier {
    private:
        int num_posts = 0;
        set<string> unique_words;
        map<string, map<string, int>> word_counts; // label -> word-> frequency
        map<string, int> label_counts; // label -> frequency keeps 
        // track of labels and how many of them we see
        set <string> get_unique_words(const string & substance){
            istringstream is(substance);
            set<string> words;
            string w;
            while(is >> w){
                words.insert(w);
            }
            return words;
        }

        bool in_set(const string & word){
            bool found = false;
            const string target = word;
            for(const string &w : unique_words){
                if (w == target){
                    found = true;
                }
            }
            return true;
        }

        double log_prior(const string &label) const {
            return log(static_cast<double>(label_counts[label]) / num_posts);
        }

        double log_likelihood(const string &word, const string &label) const {
            int posts_with_word_given_label = 0;
            int posts_with_label = label_counts[label];
            int posts_with_word = 0;
            if (word_counts[label] && word_counts.second[word]){
                posts_with_word_given_label = word_counts.second[word];
                return log(static_cast<double>(posts_with_word_given_label) / posts_with_label);
            }
            else if (!word_counts[label][word] && in_set(word)){
                for (const auto &label_entry : word_counts) {
                    if (label_entry.second[word] > 0){
                    posts_with_word += label_entry.second[word];
                    }
                }
                return log(static_cast<double>(posts_with_word) / num_posts);
            }
            else{
                return log (1.0 / num_posts);
            }

        }
        

    public:
        void train_model(const string &file){
            csvstream fin(file);
            map<string, string> row;
            while (fin >> row){
                string label = row["tag"];
                string substance = row["content"];
                set<string> words = get_unique_words(substance);
                // Update label_counts and num_posts
                label_counts[label]++;
                num_posts++;
                for(const string &word : words){
                    unique_words.insert(word);
                    word_counts[label][word]++;
                }
            }
        }
         predict(const string &file){
            csvstream fin(file);
            map<string, string> row;
            int total_posts_read = 0;
            int correct_classifications = 0;
            while (fin >> row){
                string label = row["tag"];
                string substance = row["content"];
            }
            
        }
};

int main(int argc, char *argv[]) {
  Classifier c;
  if (argc != 2 || argc != 3){
    cout << "Usage: classifier.exe TRAIN_FILE [TEST_FILE]" << endl;
    return 1;
  }
    string training_file = argv[1];
    try {
        c.train_model(training_file);
    }
    catch (const csvstream_exception &e){
        cout << "Error opening file: " << training_file << endl;
        return 1;
    }
    string test_file;
    if (argc == 3){
        test_file = argv[2];
        try {
            c.predict(test_file);
        }
        catch (const csvstream_exception &e){
            cout << "Error opening file: " << test_file << endl;
            return 1;
        }
    }
    return 0;
}
