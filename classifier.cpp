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
        map<string, int> label_counts; // label -> frequency
        vector<pair<string, string>> training_data;
        set <string> get_unique_words(const string & substance){
            istringstream is(substance);
            set<string> words;
            string w;
            while(is >> w){
                words.insert(w);
            }
            return words;
        }

        bool in_set(const string & word) const{
            bool found = false;
            const string target = word;
            for(const string &w : unique_words){
                if (w == target){
                    found = true;
                }
            }
            return found;
        }

        double log_prior(const string &label) const {
            return log(static_cast<double>(label_counts.at(label)) / num_posts);
        }

        double log_likelihood(const string &word, const string &label) const {
            int posts_with_word_given_label = 0;
            int posts_with_label = label_counts.at(label);
            int posts_with_word = 0;
            if (word_counts.count(label) && word_counts.at(label).count(word)){
                posts_with_word_given_label = word_counts.at(label).at(word);
                return log(static_cast<double>(posts_with_word_given_label) / posts_with_label);
            }
            else if (!word_counts.at(label).count(word) && in_set(word)){
                for (const auto &label_entry : word_counts) {
                    if (label_entry.second.count(word)){
                        int word_count = label_entry.second.at(word);
                        if (word_count > 0){
                        posts_with_word += word_count;
                        }
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
                training_data.emplace_back(label, substance);
                label_counts[label]++;
                num_posts++;
                for(const string &word : words){
                    unique_words.insert(word);
                    word_counts[label][word]++;
                }
            }
        }
        void predict(const string &file){
            cout << "trained on " << num_posts << " examples" << endl;
            cout << endl;
            cout << "test data:" << endl;
            csvstream fin(file);
            map<string, string> row;
            int total_posts_read = 0;
            int correct_classifications = 0;
            double best_score = -INFINITY;
            string best_label;
            while (fin >> row){
                string label = row["tag"];
                string substance = row["content"];
                double log_score = log_prior(label);
                set<string> words = get_unique_words(substance);
                for (const string &word : words){
                    log_score += log_likelihood(word, label);
                }
                if (log_score > best_score){
                    best_score = log_score;
                    best_label = label;
                }
                total_posts_read++;
                if (label == best_label){
                    correct_classifications++;
                }
                cout << "  correct = " << label << ", predicted = "
                << best_label << ", log-probability score = " << log_score << endl;
                cout << "  content = " << substance << endl;
                cout << endl;
            }
            cout << "performance: " << correct_classifications << " / "
            << total_posts_read << " posts predicted correctly" << endl;
        }
        void print_training(){
            cout << "training data:" << endl;
            
            for (const auto &[label, substance]  : training_data){
                cout << "  label = " << label << ", content = " 
                << substance << endl;
            }
            cout << "trained on " << num_posts << " examples" << endl;
            cout << "vocabulary size = " << unique_words.size() << endl;
            cout << endl;
            cout << "classes:" << endl;
            for (const auto & [label, count] : label_counts){
                cout << "  " << label << ", " << count << " examples, log-prior = "
                << log_prior(label) << endl;
            }
            cout << "classifier parameters:" << endl;
            for (const auto &[label, words] : word_counts){
                for(const auto &[word, count] : words) {
                    cout << "  " << label << ":" << word << ", count = " 
                    << count << ", log-likelihood = " 
                    << log_likelihood(word, label) << endl;;
                }
            }
            cout << endl;
        }
};

    
int main(int argc, char *argv[]) {
    cout.precision(3);
  Classifier c;
    if (argc < 2 || argc > 3){
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
    if (argc ==2 ){c.print_training();}
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
