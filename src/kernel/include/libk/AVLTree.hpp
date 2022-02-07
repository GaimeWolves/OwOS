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
		void insert(T value)
		{
			node_t *current = m_tree;
			node_t *parent = nullptr;

			while (current)
			{
				parent = current;

				if (value == current->value)
				{
					current->value = value;
					return;
				}

				if (value < current->value)
					current = current->left;
				else if (value > current->value)
					current = current->right;
			}

			current = new node_t{
				.value = value,
			    .height = 1,
			    .parent = parent,
				.left = nullptr,
				.right = nullptr,
			};

			if (parent)
			{
				if (value < parent->value)
					parent->left = current;
				else
					parent->right = current;
			}
			else
			{
				m_tree = current;
			}

			rebalance(current);

#ifdef _DEBUG_AVL_TREE
			printf_debug_msg("INSERTION\n");
			print_tree();
#endif
		}

		bool remove(T value)
		{
			return remove(find_node(value));
		}

		bool remove(const LibK::function<int(T)> &comparator)
		{
			return remove(find_node(comparator));
		}

		void traverse(const LibK::function<bool(T)> &callback)
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

		const T *find(const LibK::function<int(T)> &comparator)
		{
			node_t *node = find_node(comparator);
			return node ? &node->value : nullptr;
		}

	private:
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
					if (successor->right)
						successor->right->parent = node;
				}

				node->value = successor->value;
				node = successor->parent;
				delete successor;
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

#ifdef _DEBUG_AVL_TREE
				printf_debug_msg("DELETION %p\n", deleted);
				print_tree();
#endif
				return true;
			}

			if (node)
				rebalance(node);
			else
				rebalance(parent);

#ifdef _DEBUG_AVL_TREE
			printf_debug_msg("DELETION %p\n", deleted);
			print_tree();
#endif
			return true;
		}

		node_t *find_node(T value)
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

		node_t *find_node(const LibK::function<int(T)> &comparator)
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

#ifdef _DEBUG_AVL_TREE
		void print_tree()
		{
			if (!m_tree)
			{
				kprintf("EMPTY\n");
				return;
			}

			LibK::vector<node_t *> seen;
			LibK::vector<LibK::vector<node_t *>> slices;
			print_tree_helper(m_tree, slices, seen, 0, 0);

			int depth = slices.size() - 1;
			int width = 1 << depth;
			int max_line_width = width * 10;

			for (size_t d = 0; d < slices.size(); d++)
			{
				size_t tree_width = 1 << d;
				size_t line_width = tree_width * 10;
				size_t padding = (max_line_width - line_width) / 2;

				if (padding > 0)
					kprintf("%*c", padding, ' ');

				for (size_t i = 0; i < tree_width; i++)
				{
					node_t *node = slices[d].size() > i ? slices[d][i] : (node_t *)UINTPTR_MAX;

					if ((uintptr_t)node == UINTPTR_MAX)
						kprintf("          ");
					else if (node)
						kprintf(" %8X ", (uintptr_t)node);
					else
						kprintf("   NULL   ");
				}

				kputc('\n');
				kputc('\n');
			}
		}

		void print_tree_helper(node_t *root, LibK::vector<LibK::vector<node_t *>> &slices, LibK::vector<node_t *> &seen, int depth, size_t index)
		{
			if (slices.size() == (size_t)depth)
				slices.push_back(LibK::vector<node_t *>());

			if (slices[depth].size() < index)
			{
				while (slices[depth].size() < index)
					slices[depth].push_back((node_t *)UINTPTR_MAX);
			}

			slices[depth].push_back(root);

			if (!root)
				return;

			for (auto other : seen)
			{
				if (other == root)
					return;
			}

			seen.push_back(root);

			print_tree_helper(root->left, slices, seen, depth + 1, index * 2);
			print_tree_helper(root->right, slices, seen, depth + 1, index * 2 + 1);
		}
#endif

		node_t *m_tree{nullptr};
	};
}