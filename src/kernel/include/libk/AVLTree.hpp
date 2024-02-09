#pragma once

#include <libk/kstack.hpp>
#include <libk/kfunctional.hpp>
#include <libk/kcstdio.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class AVLTree
	{
	private:
		typedef struct node_t
		{
			T value;

			int height;

			struct node_t *parent;
			struct node_t *left;
			struct node_t *right;
		} node_t;

	public:
		AVLTree() = default;

		~AVLTree()
		{
			delete_subtree(m_tree);
			m_tree = nullptr;
		}

		AVLTree(const AVLTree &other)
		{
			this->~AVLTree();
			m_tree = copy_subtree(other.m_tree);
		}

		AVLTree(AVLTree &&other) noexcept
		{
			this->~AVLTree();
			m_tree = other.m_tree;
			other.m_tree = nullptr;
		}

		AVLTree &operator=(const AVLTree &rhs)
		{
			if (&rhs == this)
				return *this;

			this->~AVLTree();
			m_tree = copy_subtree(rhs.m_tree);

			return *this;
		}

		AVLTree &operator=(AVLTree &&rhs) noexcept
		{
			this->~AVLTree();
			m_tree = rhs.m_tree;
			rhs.m_tree = nullptr;

			return *this;
		}

		void insert(T value)
		{
			auto *node = new node_t{
			    .value = value,
			    .height = 1,
			    .parent = nullptr,
			    .left = nullptr,
			    .right = nullptr,
			};

			insert(node);
		}


		template <class ...Args>
		T *emplace(Args &&...args)
		{
			auto *node = new node_t{
			    .value = T(forward<Args>(args)...),
			    .height = 1,
			    .parent = nullptr,
			    .left = nullptr,
			    .right = nullptr,
			};

			insert(node);

			return &node->value;
		}

		bool remove(const T &value)
		{
			return remove(find_node(value));
		}

		bool remove(const LibK::function<int(T)> &comparator)
		{
			return remove(find_node(comparator));
		}

		void traverse(const LibK::function<bool(T)> &callback) const
		{
			node_t *node = m_tree;
			Kernel::LibK::stack<node_t *> nodes;

			while (!nodes.empty() || node)
			{
				if (node)
				{
					nodes.push(node);
					node = node->left;
				}
				else
				{
					node = nodes.top();
					nodes.pop();

					if (!callback(node->value))
						return;

					node = node->right;
				}
			}
		}

		const T *find(const LibK::function<int(T)> &comparator) const
		{
			node_t *node = find_node(comparator);
			return node ? &node->value : nullptr;
		}

		const T *find(const T &key) const
		{
			node_t *node = find_node(key);
			return node ? &node->value : nullptr;
		}

		T *find(const T &key)
		{
			node_t *node = find_node(key);
			return node ? &node->value : nullptr;
		}

	private:
		static void delete_subtree(node_t *tree)
		{
			if (!tree)
				return;

			delete_subtree(tree->left);
			delete_subtree(tree->right);

			delete tree;
		}

		static node_t *copy_subtree(node_t *other_tree)
		{
			if (!other_tree)
				return nullptr;

			node_t tree = new node_t {
				.parent = nullptr,
				.left = copy_subtree(other_tree->left),
				.right = copy_subtree(other_tree->right),
				.value = other_tree->value,
				.height = other_tree->height,
			};

			if (tree.left)
				tree.left->parent = tree;

			if (tree.right)
				tree.right->parent = tree;

			return tree;
		}

		void insert(node_t *node)
		{
			node_t *current = m_tree;
			node_t *parent = nullptr;

			while (current)
			{
				parent = current;

				if (node->value < current->value)
					current = current->left;
				else if (current->value < node->value)
					current = current->right;
				else
					assert(false);
			}

			current = node;
			current->parent = parent;

			if (parent)
			{
				if (current->value < parent->value)
					parent->left = current;
				else
					parent->right = current;
			}
			else
			{
				m_tree = current;
			}

			rebalance(current);
		}

		bool remove(node_t *node)
		{
			if (!node)
				return false;

			auto parent = node->parent;

			if (node->left && node->right)
			{
				auto *successor = node->right;
				while (successor->left)
					successor = successor->left;

				if (successor->parent != node)
				{
					successor->parent->left = successor->right;
					if (successor->right)
						successor->right->parent = successor->parent;
				}
				else
				{
					node->right = successor->right;
				}

				node_t *new_node = successor->parent == node ? successor : successor->parent;

				successor->left = node->left;
				node->left->parent = successor;

				successor->right = node->right;
				if (node->right)
					node->right->parent = successor;

				successor->parent = node->parent;

				if (node->parent->left == node)
				{
					node->parent->left = successor;
				}
				else
				{
					node->parent->right = successor;
				}

				delete node;
				node = new_node;
			}
			else
			{
				node_t *next = node->left ? node->left : node->right;

				if (next)
					next->parent = parent;

				if (parent)
				{
					if (node->parent->left == node)
						parent->left = next;
					else
						parent->right = next;
				}

				delete node;
				node = next;
			}

			if (!node && !parent)
			{
				m_tree = nullptr;
				return true;
			}

			if (node)
				rebalance(node);
			else
				rebalance(parent);

			return true;
		}

		node_t *find_node(const T &value) const
		{
			node_t *current = m_tree;

			while(current)
			{
				if (current->value == value)
					return current;
				else if (current->value < value)
					current = current->right;
				else
					current = current->left;
			}

			return nullptr;
		}

		node_t *find_node(const LibK::function<int(T)> &comparator) const
		{
			node_t *current = m_tree;

			while(current)
			{
				if (comparator(current->value) == 0)
					return current;
				else if (comparator(current->value) < 0)
					current = current->right;
				else
					current = current->left;
			}

			return current;
		}

		void rebalance(node_t *node)
		{
			while (node)
			{
				update_height(node);
				int bf = get_balance_factor(node);

				if (bf < -1)
				{
					if (get_balance_factor(node->left) <= -1)
						node = rotate_right(node);
					else
						node = rotate_left_right(node);
				}
				else if (bf > 1)
				{
					if (get_balance_factor(node->right) >= 1)
						node = rotate_left(node);
					else
						node = rotate_right_left(node);
				}

				if (!node->parent)
					m_tree = node;

				node = node->parent;
			}
		}

		node_t *rotate_right(node_t *root)
		{
			auto child = root->left;
			auto middle = child->right;

			root->left = middle;
			child->right = root;

			if (middle)
				middle->parent = root;

			if (root->parent)
			{
				if (root->parent->left == root)
					root->parent->left = child;
				else
					root->parent->right = child;
			}

			child->parent = root->parent;
			root->parent = child;

			update_height(root);
			update_height(child);

			return child;
		}

		node_t *rotate_left(node_t *root)
		{
			auto child = root->right;
			auto middle = child->left;

			root->right = middle;
			child->left = root;

			if (middle)
				middle->parent = root;

			if (root->parent)
			{
				if (root->parent->left == root)
					root->parent->left = child;
				else
					root->parent->right = child;
			}

			child->parent = root->parent;
			root->parent = child;

			update_height(root);
			update_height(child);

			return child;
		}

		node_t *rotate_right_left(node_t *root)
		{
			root->right = rotate_right(root->right);
			return rotate_left(root);
		}

		node_t *rotate_left_right(node_t *root)
		{
			root->left = rotate_left(root->left);
			return rotate_right(root);
		}

		void update_height(node_t *root)
		{
			int left = root->left ? root->left->height : 0;
			int right = root->right ? root->right->height : 0;

			root->height = LibK::max(left, right) + 1;
		}

		int get_balance_factor(node_t *root)
		{
			int left = root->left ? root->left->height : 0;
			int right = root->right ? root->right->height : 0;

			return right - left;
		}

		node_t *m_tree{nullptr};
	};
}