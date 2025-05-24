#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

const int MAXN = 200005;

// Represents a node in the splay tree.
struct Node {
    int pa;   // Parent node ID
    int ch[2]; // Left (0) and Right (1) child IDs
    int key;  // Value of the current element
    int sum;  // Sum of elements in the subtree rooted at this node
    int lazy; // Additive lazy tag for range updates
    int sz;   // Size of the subtree rooted at this node (including itself)

    Node() : pa(0), key(0), sum(0), lazy(0), sz(0) {
        ch[0] = ch[1] = 0;
    }
};


Node tree[MAXN];  // Array to store all nodes in the splay tree
int root;         // Root of the splay tree
int tot_nodes;    // Total nodes allocated in the tree array

// --- Core Splay Tree Operations ---

// Updates the size and sum of node x based on its children's information.
void push_up(int x) {
    if (!x) return;
    tree[x].sz = tree[tree[x].ch[0]].sz + tree[tree[x].ch[1]].sz + 1;
    tree[x].sum = tree[tree[x].ch[0]].sum + tree[tree[x].ch[1]].sum + tree[x].key;
}

// Apply lazy tag value to node x
void apply_lazy_value(int x, int val) {
    if (!x) return;
    tree[x].key += val;
    tree[x].sum += val * tree[x].sz;
    tree[x].lazy += val;
}

// Pushes down the lazy tag from node x to its children.
void push_down(int x) {
    if (!x || tree[x].lazy == 0) return;
    if (tree[x].ch[0]) apply_lazy_value(tree[x].ch[0], tree[x].lazy);
    if (tree[x].ch[1]) apply_lazy_value(tree[x].ch[1], tree[x].lazy);
    tree[x].lazy = 0;
}

// Get which child x is of its parent (0 for left, 1 for right)
int get_child_type(int x) {
    return tree[tree[x].pa].ch[1] == x;
}

// Rotate node x up one level
void rotate(int x) {
    int y = tree[x].pa;
    int z = tree[y].pa;
    int x_type = get_child_type(x);
    int y_type = get_child_type(y);

    if (z) tree[z].ch[y_type] = x;
    tree[x].pa = z;

    tree[y].ch[x_type] = tree[x].ch[x_type ^ 1];
    if (tree[x].ch[x_type ^ 1]) tree[tree[x].ch[x_type ^ 1]].pa = y;

    tree[x].ch[x_type ^ 1] = y;
    tree[y].pa = x;

    push_up(y);
    push_up(x);
}

// Splay node x to be a child of 'goal_pa' (or root if goal_pa is 0)
void splay(int x, int goal_pa = 0) {
    while (tree[x].pa != goal_pa) {
        int y = tree[x].pa;
        int z = tree[y].pa;

        if (z != goal_pa) push_down(z); 
        push_down(y);                   
        push_down(x);


        if (z == goal_pa) { // Zig step
            rotate(x);
        } else {
            if (get_child_type(x) == get_child_type(y)) { // Zig-Zig step
                rotate(y);
                rotate(x);
            } else { // Zig-Zag step
                rotate(x);
                rotate(x);
            }
        }
    }
    if (goal_pa == 0) {
        root = x;
    }
}


// Creates a new node and returns its ID
int new_node(int key_val, int parent_node) {
    tot_nodes++;
    tree[tot_nodes].pa = parent_node;
    tree[tot_nodes].ch[0] = tree[tot_nodes].ch[1] = 0;
    tree[tot_nodes].key = key_val;
    tree[tot_nodes].sum = key_val;
    tree[tot_nodes].lazy = 0;
    tree[tot_nodes].sz = 1;
    return tot_nodes;
}

// Build tree recursively from a segment of the input array
// arr is 0-indexed. l_idx, r_idx are indices into arr.
// Returns the ID of the root of the built subtree.
int build_recursive(const vector<int>& arr, int l_idx, int r_idx, int parent_node) {
    if (l_idx > r_idx) return 0;
    int mid_idx = l_idx + (r_idx - l_idx) / 2;
    int curr_node = new_node(arr[mid_idx], parent_node);
    
    tree[curr_node].ch[0] = build_recursive(arr, l_idx, mid_idx - 1, curr_node);
    tree[curr_node].ch[1] = build_recursive(arr, mid_idx + 1, r_idx, curr_node);
    
    push_up(curr_node);
    return curr_node;
}

// Finds the k-th node in the splay tree (1-indexed based on current tree structure including dummies)
// Does NOT splay the found node; caller is responsible for splaying if needed.
int find_kth(int k_rank) {
    int curr = root;
    if (k_rank < 1 || k_rank > tree[root].sz) return 0; 

    while (true) {
        push_down(curr);
        int left_sz = tree[tree[curr].ch[0]].sz;
        if (k_rank <= left_sz) {
            curr = tree[curr].ch[0];
        } else if (k_rank == left_sz + 1) {
            return curr;
        } else {
            k_rank -= (left_sz + 1);
            curr = tree[curr].ch[1];
        }
    }
}

// Helper to isolate the subtree for an original 0-indexed range [l_orig, r_orig].
// It splays nodes such that the root of the desired subtree is tree[tree[root].ch[1]].ch[0].
// Returns the ID of this subtree root.
// Tree ranks for boundaries:
// Node before a[l_orig] (i.e., a[l_orig-1] or DUMMY_MIN) is at tree rank l_orig + 1.
// Node after  a[r_orig] (i.e., a[r_orig+1] or DUMMY_MAX) is at tree rank r_orig + 3.
int get_interval_subtree_root(int l_orig, int r_orig) {
    int left_boundary_node = find_kth(l_orig + 1); 
    splay(left_boundary_node, 0);

    int right_boundary_node = find_kth(r_orig + 3); 
    splay(right_boundary_node, root);

    return tree[right_boundary_node].ch[0]; 
}


/**
 * @brief Builds the splay tree from an initial sequence of integers.
 * Clears any existing tree structure.
 * The sequence is represented by nodes between two dummy nodes.
 *
 * @note Time Complexity: O(N), Space Complexity: O(N) where N is the size of the initial sequence.
 *
 * @param initial_sequence The sequence of integers to build the tree from.
 */
void build_from_sequence(const vector<int>& initial_sequence) {
    tot_nodes = 0;
    // Tree[0] is a sentinel/null node, its size should always be 0.
    tree[0].sz = 0; tree[0].sum = 0; tree[0].key = 0; tree[0].lazy = 0;

    // Dummy node at the beginning (tree rank 1)
    root = new_node(0, 0); 
    
    // Dummy node at the end (tree rank N+2, where N is initial_sequence.size())
    tree[root].ch[1] = new_node(0, root); 

    int actual_data_root = build_recursive(initial_sequence, 0, 
                                           initial_sequence.empty() ? -1 : (int)initial_sequence.size() - 1, 
                                           tree[root].ch[1]);
    tree[tree[root].ch[1]].ch[0] = actual_data_root;

    push_up(tree[root].ch[1]);
    push_up(root);
}


/**
 * @brief Inserts a new element with value `val` at 0-indexed `pos` in the sequence.
 *
 * @param pos The 0-indexed position where the element should be inserted.
 * @param val The value of the element to insert.
 *
 * @note Time Complexity: O(log N) amortized.
 */
void insert_at_position(int pos, int val) {
    int prev_node = find_kth(pos + 1); // Node that will be before the new value
    splay(prev_node, 0);

    int next_node = find_kth(pos + 2); // Node that will be after the new value
    splay(next_node, root);

    int new_val_node = new_node(val, next_node);
    tree[next_node].ch[0] = new_val_node;

    push_up(next_node);
    push_up(root);
}

/**
 * @brief Deletes the element at 0-indexed `pos` from the sequence.
 *
 * @param pos The 0-indexed position of the element to delete.
 *
 * @note Time Complexity: O(log N) amortized.
 */
void delete_at_position(int pos) {
    int prev_node = find_kth(pos + 1); // Node before the one to delete
    splay(prev_node, 0);

    int next_node = find_kth(pos + 3); // Node after the one to delete
    splay(next_node, root);

    tree[next_node].ch[0] = 0;

    push_up(next_node);
    push_up(root);
}


/**
 * @brief Updates the values of elements in the sequence range [l, r] (0-indexed) by adding `val_to_add`.
 * This is a range update operation using lazy propagation.
 *
 * @param l The 0-indexed start of the range (inclusive).
 * @param r The 0-indexed end of the range (inclusive).
 * @param val_to_add The value to add to each element in the range.
 *
 * @note Time Complexity: O(log N) amortized.
 */
void update_range(int l, int r, int val_to_add) {
    if (l > r) return;
    int subtree_r = get_interval_subtree_root(l, r);
    apply_lazy_value(subtree_r, val_to_add);
    
    push_up(tree[subtree_r].pa); 
    push_up(tree[tree[subtree_r].pa].pa); 
}

/**
 * @brief Queries the sum of elements in the sequence range [l, r] (0-indexed).
 *
 * @param l The 0-indexed start of the range (inclusive).
 * @param r The 0-indexed end of the range (inclusive).
 * @return The sum of elements in the specified range. Returns 0 for an empty range (l > r).
 *
 * @note Time Complexity: O(log N) amortized.
 */
int query_sum_range(int l, int r) {
    if (l > r) return 0;
    int subtree_r = get_interval_subtree_root(l, r);
    return tree[subtree_r].sum;
}

void run_tests() {
    cout << "--- Running Splay Tree Tests ---" << endl;
    vector<int> model; 

    // Test Case 1: Basic Build and Query
    cout << "\nTest Case 1: Basic Build and Query" << endl;
    model = {10, 20, 30, 40, 50};
    build_from_sequence(model);
    assert(query_sum_range(0, 4) == 150);
    assert(query_sum_range(1, 3) == 90);
    assert(query_sum_range(2, 2) == 30); 
    assert(query_sum_range(2, 1) == 0);

    // Test Case 2: Insertions
    cout << "\nTest Case 2: Insertions" << endl;
    model = {10, 20, 30};
    build_from_sequence(model);

    insert_at_position(1, 15);
    assert(query_sum_range(0, 3) == 75);
    assert(query_sum_range(1, 1) == 15);

    insert_at_position(0, 5);
    assert(query_sum_range(0, 4) == 80);
    assert(query_sum_range(0, 0) == 5);
    
    model.push_back(40);
    insert_at_position(5, 40); 
    assert(query_sum_range(0, 5) == 120);
    assert(query_sum_range(5, 5) == 40);

    // Test Case 3: Deletions
    cout << "\nTest Case 3: Deletions" << endl;
    model = {10, 20, 30, 40, 50};
    build_from_sequence(model);

    delete_at_position(2);
    assert(query_sum_range(0, 3) == 120);
    assert(query_sum_range(1, 2) == 60);

    delete_at_position(0);
    assert(query_sum_range(0, 2) == 110);
    assert(query_sum_range(0, 0) == 20);

    delete_at_position(2); 
    assert(query_sum_range(0, 1) == 60);
    assert(query_sum_range(1, 1) == 40);

    // Test Case 4: Updates
    cout << "\nTest Case 4: Updates" << endl;
    model = {10, 20, 30, 40, 50};
    build_from_sequence(model);

    update_range(1, 3, 5);
    assert(query_sum_range(0, 4) == 165);
    
    update_range(0, 4, -10);
    assert(query_sum_range(0, 4) == 115);

    update_range(2, 2, 100);
    assert(query_sum_range(2, 2) == 125);
    assert(query_sum_range(0, 4) == 215);

    // Test Case 5: Mixed Operations & Empty Sequence
    cout << "\nTest Case 5: Mixed Operations & Empty Sequence" << endl;
    model.clear();
    build_from_sequence(model); 
    assert(query_sum_range(0, -1) == 0); 

    insert_at_position(0, 10);
    assert(query_sum_range(0, 0) == 10);

    insert_at_position(1, 20);
    assert(query_sum_range(0, 1) == 30);

    insert_at_position(0, 5);
    assert(query_sum_range(0, 2) == 35);

    update_range(0, 1, 1);
    assert(query_sum_range(0, 2) == 37);

    delete_at_position(1);
    assert(query_sum_range(0, 1) == 26);
    
    delete_at_position(1);
    assert(query_sum_range(0, 0) == 6);

    delete_at_position(0);
    assert(query_sum_range(0, -1) == 0); 

    insert_at_position(0, 100);
    assert(query_sum_range(0, 0) == 100);

    cout << "\n--- All tests passed! ---" << endl;
}

void run_splay_tree_sample() {
    vector<int> initial_data = {10, 20, 30, 40, 50};
    build_from_sequence(initial_data); 

    cout << "Sum of [1, 3] (20,30,40): " << query_sum_range(1, 3) << endl;

    update_range(1, 3, 5);
    cout << "Sum of [1, 3] (25,35,45): " << query_sum_range(1, 3) << endl;
    cout << "Sum of [0, 4] (10,25,35,45,50): " << query_sum_range(0, 4) << endl;
    
    insert_at_position(2, 100);
    cout << "Sum of [0, 5]: " << query_sum_range(0, 5) << endl;
    cout << "Sum of [2, 4] (100,35,45): " << query_sum_range(2, 4) << endl;

    delete_at_position(3);
    cout << "Sum of [0, 4]: " << query_sum_range(0, 4) << endl;
    cout << "Sum of [2, 3] (100,45): " << query_sum_range(2, 3) << endl;

    update_range(0, 4, -10);
    cout << "Sum of [0, 4]: " << query_sum_range(0, 4) << endl;
    
    insert_at_position(0, 999);
    cout << "Sum of [0,0] (999): " << query_sum_range(0,0) << endl;
    cout << "Sum of [0, 5]: " << query_sum_range(0, 5) << endl;

    int current_num_elements = tree[tree[tree[root].ch[1]].ch[0]].sz;
    insert_at_position(current_num_elements, 888);
    cout << "Sum of ["<< current_num_elements << "," << current_num_elements << "] (888): " << query_sum_range(current_num_elements,current_num_elements) << endl;
    current_num_elements++;
    cout << "Sum of [0, " << current_num_elements-1 << "]: " << query_sum_range(0, current_num_elements-1) << endl;
    
    delete_at_position(0);
    current_num_elements--;
    cout << "Sum of [0, " << current_num_elements-1 << "]: " << query_sum_range(0, current_num_elements-1) << endl;

    delete_at_position(current_num_elements-1);
    current_num_elements--;
    cout << "Sum of [0, " << current_num_elements-1 << "]: " << query_sum_range(0, current_num_elements-1) << endl;
    
}


int main() {
    run_tests();
    run_splay_tree_sample();
    return 0;
}