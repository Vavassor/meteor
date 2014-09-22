#ifndef LINKED_LIST_H
#define LINKED_LIST_H

template <typename T>
class LinkedList
{
private:
	struct Node
	{
		T value;
		Node* next;
	};

	Node* head;
	Node* tail;

public:
	LinkedList():
		head(nullptr),
		tail(nullptr)
	{}

	~LinkedList()
	{
		Node* node = tail;
		while(node != nullptr)
		{
			Node* temp = node->next;
			delete node;
			node = temp;
		}
	}

	void Add(const T& value)
	{
		Node* node = new Node;
		node->value = value;
		node->next = nullptr;

		if(head != nullptr)
			head->next = node;
		else
			tail = node;

		head = node;
	}

	template <typename EqualFunctor>
	void Remove(const T& value, const EqualFunctor& equals)
	{
		Node* last = nullptr;

		Node* node = tail;
		while(node != nullptr)
		{
			if(equals(node->value, value))
			{
				if(last != nullptr)
					last->next = node->next;
				else // implies if(node == tail)
					tail = node->next;
				if(node == head)
					head = nullptr;

				delete node;
				break;
			}
			last = node;
			node = node->next;
		}
	}

	class ConstIterator;

	ConstIterator First()
	{
		return ConstIterator(tail);
	}

	ConstIterator Last()
	{
		return ConstIterator(head->next);
	}

	friend class ConstIterator;
	class ConstIterator
	{
	private:
		Node* current;

	public:
		ConstIterator(Node* node):
			current(node)
		{}

		ConstIterator(const ConstIterator& other):
			current(other.current)
		{}

		const T& operator * () const { return current->value; }
		const T* operator -> () const { return &current->value; }
		bool operator != (const ConstIterator& other) const { return current != other.current; }
		bool operator == (const ConstIterator& other) const { return current == other.current; }

		ConstIterator& operator++()
		{
			current = current->next;
			return *this;
		}
	};
};

#endif
