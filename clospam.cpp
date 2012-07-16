// CloSpam -- CLOSed PAttern Mining
//
// Leslie Wu
//
// CloSpam implements CLOSET by Jian Pei, Jiawei Han, and Runying Mao
//

#include <iostream>
#include <fstream>
#include "transaction_db.h"
#include "fpmine.h"
#include "fptree.h"

bool g_debug = false; // Debugging flag

using namespace std;

void usage()
{
	cerr << 
		"\n"
		"usage:\n"
		"\t clospam test.data minsup"
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc == 1) {
		usage();
		return 1;
	}	

	cerr << "\n[CloSpam] by Leslie Wu\n";

	// Determine minsup, or use default
	int minsup = 2;

	if (argc > 2) {
		const char* minsup_str = argv[2];
		minsup = atoi(minsup_str);

		cerr << "Setting minsup = " << minsup << endl;

		if (argc > 3) {
			cout << "Debug output enabled." << endl;
			g_debug = true;
		}
	} else {
		cout << "Using default minsup = " << minsup << endl;
	}

	// Read file
	const char* fn = argv[1];
	cerr << "Reading [" << fn << "]... " << flush;

	TransactionDB db;

	if (db.open(fn)) {
		cerr << "done.\n" << endl;

		// Debugging:
		if (g_debug) {
			cout << "Transaction database:" << endl;
			db.dump_trans(cout);

			cout << "Read " << ((int)db.data.size()-1) << " transactions.\n" << endl;

			cout << "Vertical format (tID mapping):" << endl;
			db.dump_indices(cout);		
		}
	} else {
		cerr << "failed." << endl;
		return 1;
	}

	if (g_debug) {
		// Write out to CLOSET-compatible format
		db.write_out_to_closet("ibm.closet");
	}

	// Find frequent single items
	FrequencyVec f_list;
	find_frequent_items(db, f_list, minsup);

	// Sort each tuple
	sort_tuples_by_frequency(db, f_list);

	// Construct FP-tree
	FPtree fpee;
	fpee.build(db, f_list);

	if (g_debug) fpee.print(cout);

	// Mine
	fpee.close_mine(f_list, minsup);


	// Write frequent patterns to disk
	const char* frequent_fn = "frequent.dat";
	cerr << "Writing frequent patterns to [" << frequent_fn << "]... "; 
	ofstream outfile(frequent_fn);
	fpee.print_frequent_patterns(outfile);
	cout << "done.\n" << endl;

	// Print summary table
	fpee.print_frequent_pattern_summary(cout);

	return 0;
}
