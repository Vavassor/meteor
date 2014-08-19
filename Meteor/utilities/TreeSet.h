#ifndef TREE_SET_H
#define TREE_SET_H

template<typename Key>
class TreeSet
{
private:
	class BNode
	{
	private:
		Key* keys;
		int t; // Minimum degree (defines the range for number of keys)
		BNode** children;
		int numKeys;
		bool leaf;

		int FindIndex(const Key& k)
		{
			int i = 0;
			while(i < numKeys && keys[i] < k) ++i;
			return i;
		}

		void RemoveFromNonLeaf(int idx)
		{
			if(children[idx]->numKeys >= t)
			{
				// If the child that precedes k (C[idx]) has at least t keys,
				// find the predecessor of k in the subtree rooted at C[idx]
				BNode* cur = children[idx];
				while(!cur->leaf)
					cur = cur->children[cur->numKeys];

				// set k to predecessor and remove from subtree
				Key predecessor = cur->keys[cur->numKeys - 1];
				keys[idx] = predecessor;
				children[idx]->Remove(predecessor);
			}
			else if(children[idx + 1]->numKeys >= t)
			{
				// else, since the child C[idx] has less than t keys, examine C[idx + 1].
				// If C[idx+1] has at least t keys, find the successor of k in
				// the subtree rooted at C[idx + 1].
				BNode* cur = children[idx + 1];
				while(!cur->leaf)
					cur = cur->children[0];

				// set k to successor and remove from subtree
				Key successor = cur->keys[0];
				keys[idx] = successor;
				children[idx + 1]->Remove(successor);
			}
			else
			{
				// If both have less than t keys, merge k and all of C[idx+1] into C[idx]
				// then C[idx] will contain 2t - 1 keys
				Merge(idx);
				children[idx]->Remove(keys[idx]);
			}
		}

		void RotateFromLeft(int idx)
		{
			BNode* child = children[idx];
			BNode* sibling = children[idx - 1];
 
			// the sibling loses one key and child gains one key
			for(int i = child->numKeys - 1; i >= 0; --i)
				child->keys[i + 1] = child->keys[i];
 
			if(!child->leaf)
			{
				for(int i = child->numKeys; i >= 0; --i)
					child->children[i + 1] = child->children[i];
			}
 
			child->keys[0] = keys[idx - 1];
 
			// Moving sibling's last child as children[idx]'s first child
			if(!leaf) child->children[0] = sibling->children[sibling->numKeys];
 
			// Moving the key from the sibling to the parent
			keys[idx - 1] = sibling->keys[--sibling->numKeys];
 
			++child->numKeys;
		}

		void RotateFromRight(int idx)
		{
			BNode* child = children[idx];
			BNode* sibling = children[idx + 1];
 
			// keys[idx] is inserted as the last key in child
			child->keys[child->numKeys] = keys[idx];
 
			// sibling's first child is inserted as the last child of this node
			if(!child->leaf) child->children[child->numKeys + 1] = sibling->children[0];
			keys[idx] = sibling->keys[0];
 
			// shift sibling's keys and their children down
			for(int i = 1; i < sibling->numKeys; ++i)
				sibling->keys[i - 1] = sibling->keys[i];

			if(!sibling->leaf)
			{
				for(int i = 1; i <= sibling->numKeys; ++i)
					sibling->children[i - 1] = sibling->children[i];
			}

			++child->numKeys;
			--sibling->numKeys;
		}

		void Merge(int idx)
		{
			// merges right into left
			BNode* left = children[idx];
			BNode* right = children[idx + 1];
 
			left->keys[t - 1] = keys[idx];
 
			// copy keys and children from right to left child
			for(int i = 0; i < right->numKeys; ++i)
				left->keys[i + t] = right->keys[i];
 
			if(!left->leaf)
			{
				for(int i = 0; i <= right->numKeys; ++i)
					left->children[i + t] = right->children[i];
			}

			// shift children and keys of this node to fill the leftover space
			for(int i = idx + 1; i < numKeys; ++i)
				keys[i - 1] = keys[i];
			for(int i = idx + 2; i <= numKeys; ++i)
				children[i - 1] = children[i];
 
			left->numKeys += right->numKeys + 1;
			--numKeys;
 
			delete right;
		}

		void Fill(int idx)
		{
			if(idx != 0 && children[idx - 1]->numKeys >= t)
				RotateFromLeft(idx);
			else if (idx != numKeys && children[idx + 1]->numKeys >= t)
				RotateFromRight(idx);
			else
			{
				// If children[idx] is the last child, merge it with with its previous sibling
				// Otherwise merge it with its next sibling
				Merge((idx != numKeys) ? idx : idx - 1);
			}
		}

	public:
		BNode(int t, bool leaf):
			t(t),
			leaf(leaf),
			numKeys(0)
		{
			keys = new Key[2 * t - 1];
			children = new BNode*[2 * t];
		}

		~BNode()
		{
			delete[] keys;
			for(int i = 0; i < 2 * t; ++i)
				delete children[i];
			delete[] children;
		}
 
		void Traverse()
		{
			int i = 0;
			while(i < numKeys)
			{
				if(!leaf) children[i++]->Traverse();
				// Do a thing
			}
			if(!leaf) children[i]->Traverse();
		}

		BNode* Search(const Key& k)
		{
			int i = FindIndex(k);
			if(keys[i] == k) return this;
			if(leaf == true) return nullptr;

			// if not found, try smallest matching child
			return children[i]->Search(k);
		}

		void Insert(const Key& k)
		{
			int i = numKeys - 1;
			if(leaf)
			{
				// shift up all greater keys
				// and place k
				while(i >= 0 && keys[i] > k)
				{
					keys[i + 1] = keys[i];
					--i;
				}

				keys[i + 1] = k;
				++numKeys;
			}
			else
			{
				// search keys for matching child
				while(i >= 0 && keys[i] > k) i--;
 
				if(children[i + 1]->numKeys == 2 * t - 1)
				{
					// if the child is full, split it and
					// determine which side to put the key in
					SplitChild(i + 1, children[i + 1]);
					if(keys[i + 1] < k) ++i;
				}

				children[i + 1]->Insert(k);
			}
		}

		void SplitChild(int i, BNode* y)
		{
			// Create a node and give it half (t-1) of y's keys
			BNode* z = new BNode(y->t, y->leaf);

			z->numKeys = t - 1;
			for(int j = 0; j < t - 1; ++j)
				z->keys[j] = y->keys[j + t];
 
			if(!y->leaf)
			{
				for(int j = 0; j < t; ++j)
					z->children[j] = y->children[j + t];
			}
 
			y->numKeys = t - 1;
 
			// shift children to add z as a sibling alongside y
			for(int j = numKeys; j >= i + 1; --j)
				children[j + 1] = children[j];
			children[i + 1] = z;
 
			for(int j = numKeys - 1; j >= i; --j)
				keys[j + 1] = keys[j];
			keys[i] = y->keys[t - 1];
 
			++numKeys;
		}

		void Remove(const Key& k)
		{
			int idx = FindIndex(k);
 
			// The key to be removed is in this node
			if(idx < numKeys && keys[idx] == k)
			{
				if(leaf) 
				{
					// remove key from leaf by shifting all keys ahead, downward
					for(int i = idx + 1; i < numKeys; ++i)
						keys[i - 1] = keys[i];
					--numKeys;
				}
				else RemoveFromNonLeaf(idx);
			}
			else
			{
				// If this node is a leaf, then the key to be removed was not found
				if(leaf) return;
 
				// If the child where the key is supposed to exist has less than t keys,
				// fill that child to balance the tree
				if(children[idx]->numKeys < t)
					Fill(idx);
 
				// If the last child has been merged, it must have merged with the previous
				// child and so we recurse on the (idx-1)th child. Else, we recurse on the
				// (idx)th child which now has at least t keys
				if(idx == numKeys && idx > numKeys)
					children[idx - 1]->Remove(k);
				else
					children[idx]->Remove(k);
			}
		}
	};

	BNode* root;
	int t;

public:
	TreeSet(int t):
		root(nullptr),
		t(t)
	{}

	~TreeSet()
	{
		delete root;
	}

	void Insert(Key k)
	{
		if(root == nullptr)
		{
			root = new BNode(t, true);
			root->keys[0] = k;
			root->numKeys = 1;
		}
		else
		{
			// If root is full, then tree grows in height
			if(root->numKeys == 2 * t - 1)
			{
				BNode* s = new BNode(t, false);
 
				// Make old root a child of new root
				s->children[0] = root;
 
				// Split the old root and move 1 key to the new root
				s->SplitChild(0, root);
 
				// New root has two children now. Decide which of the
				// two children is going to have new key
				int i = 0;
				if(s->keys[0] < k) i++;
				s->children[i]->Insert(k);
 
				root = s;
			}
			// else insert into root
			else
			{
				root->Insert(k);
			}
		}
	}

	void Remove(const Key& k)
	{
		if(root == nullptr) return;
		root->Remove(k);
 
		// If the root node has 0 keys, make its first child as the new root
		// if it has a child, otherwise make it null
		if(root->numKeys == 0)
		{
			BNode* tmp = root;
			root = (root->leaf) ? nullptr : root->children[0];
			delete tmp;
		}
	}

	bool Contains(const Key& k) const
	{
		if(root == nullptr) return false;
		return root->Search(k) != nullptr;
	}
};

#endif
