//------------------------//------------------------
// Contents(処理内容) 敵AIで使うビヘイビアツリーの基本ノード群を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
//-----------------------------------------------------------------------------
// BehaviorTree
//-----------------------------------------------------------------------------
// 役割:
// - 敵AIの意思決定を木構造へ分離し、条件追加時の分岐爆発を抑える。
// - Sequence / Selector / Condition / Action の最小構成を提供する。
//-----------------------------------------------------------------------------
#include <functional>
#include <memory>
#include <vector>
namespace Action
{
	namespace AI
	{
		enum class NodeStatus
		{
			Success,
			Failure,
			Running
		};
		template <typename TContext>
		class Node
		{
		public:
			// 基底ノードを派生クラス経由で安全に破棄できるようにする。
			virtual ~Node() {}
			// 現在の文脈を評価してノード状態を返す。
			virtual NodeStatus Tick(TContext& context) = 0;
		};
		template <typename TContext>
		using NodePtr = std::shared_ptr<Node<TContext>>;
		template <typename TContext>
		class ConditionNode : public Node<TContext>
		{
		public:
			// 条件判定関数を保持して条件ノードを構築する。
			explicit ConditionNode(const std::function<bool(const TContext&)>& predicate)
				: m_predicate(predicate)
			{
			}
			// 条件式を評価し、成功か失敗を返す。
			NodeStatus Tick(TContext& context) override
			{
				if (!m_predicate)
				{
					return NodeStatus::Failure;
				}
				return m_predicate(context) ? NodeStatus::Success : NodeStatus::Failure;
			}
		private:
			std::function<bool(const TContext&)> m_predicate;
		};
		template <typename TContext>
		class ActionNode : public Node<TContext>
		{
		public:
			// 実行関数を保持してアクションノードを構築する。
			explicit ActionNode(const std::function<NodeStatus(TContext&)>& action)
				: m_action(action)
			{
			}
			// 実行関数を呼び出して結果状態を返す。
			NodeStatus Tick(TContext& context) override
			{
				if (!m_action)
				{
					return NodeStatus::Failure;
				}
				return m_action(context);
			}
		private:
			std::function<NodeStatus(TContext&)> m_action;
		};
		template <typename TContext>
		class SequenceNode : public Node<TContext>
		{
		public:
			// 順番に評価する子ノードを追加する。
			void AddChild(const NodePtr<TContext>& child)
			{
				if (child)
				{
					m_children.push_back(child);
				}
			}
			// 子ノードを順番に評価し、最初の失敗か実行中を返す。
			NodeStatus Tick(TContext& context) override
			{
				for (size_t i = 0; i < m_children.size(); ++i)
				{
					const NodeStatus status = m_children[i]->Tick(context);
					if (status != NodeStatus::Success)
					{
						return status;
					}
				}
				return NodeStatus::Success;
			}
		private:
			std::vector<NodePtr<TContext>> m_children;
		};
		template <typename TContext>
		class SelectorNode : public Node<TContext>
		{
		public:
			// 候補として評価する子ノードを追加する。
			void AddChild(const NodePtr<TContext>& child)
			{
				if (child)
				{
					m_children.push_back(child);
				}
			}
			// 子ノードを順番に評価し、最初の成功か実行中を返す。
			NodeStatus Tick(TContext& context) override
			{
				for (size_t i = 0; i < m_children.size(); ++i)
				{
					const NodeStatus status = m_children[i]->Tick(context);
					if (status == NodeStatus::Success || status == NodeStatus::Running)
					{
						return status;
					}
				}
				return NodeStatus::Failure;
			}
		private:
			std::vector<NodePtr<TContext>> m_children;
		};
	}
}

