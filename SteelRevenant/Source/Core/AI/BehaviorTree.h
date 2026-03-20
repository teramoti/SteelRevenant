#pragma once

//=============================================================================
// BehaviorTree.h
//
// 【役割】
//   敵 AI の意思決定を木構造で記述するための汎用ノード群を提供する。
//   CombatSystem 内の if-else チェーンを宣言的な木に置き換えることで、
//   新しい行動タイプを追加するときの「分岐爆発」を防ぐ。
//
// 【設計パターン】
//   - Composite（GoF）:
//       Node<TContext> を基底とし、
//       SequenceNode / SelectorNode が子ノードを保持・評価する。
//       葉ノードは ConditionNode（条件判定）と ActionNode（実行）の 2 種。
//
//   - テンプレート設計:
//       TContext を型引数にすることで、
//       このヘッダを敵 AI 以外の用途（味方 AI、トラップ AI など）にも
//       再利用できる汎用実装になっている。
//
// 【ノードの戻り値】
//   NodeStatus::Success  - 評価/実行が成功した
//   NodeStatus::Failure  - 評価/実行が失敗した
//   NodeStatus::Running  - 実行が継続中（複数フレームにわたる処理）
//
// 【使い方（擬似コード）】
//   auto root = std::make_shared<SelectorNode<EnemyCtx>>();
//   auto chaseSeq = std::make_shared<SequenceNode<EnemyCtx>>();
//   chaseSeq->AddChild(make_shared<ConditionNode<EnemyCtx>>(isPlayerNear));
//   chaseSeq->AddChild(make_shared<ActionNode<EnemyCtx>>(doChase));
//   root->AddChild(chaseSeq);
//   root->Tick(context); // 毎フレーム呼ぶ
//=============================================================================

#include <functional>
#include <memory>
#include <vector>

namespace Core
{
    namespace AI
    {
        //=====================================================================
        // NodeStatus  ― ノード評価の戻り値
        //=====================================================================
        enum class NodeStatus
        {
            Success, ///< 評価・実行が成功した
            Failure, ///< 評価・実行が失敗した
            Running  ///< 実行が継続中
        };

        //=====================================================================
        // Node<TContext>  ― 全ノードの基底クラス (Composite パターン)
        //=====================================================================
        template <typename TContext>
        class Node
        {
        public:
            /// @brief 派生クラスを基底ポインタ経由で安全に破棄できるようにする。
            virtual ~Node() = default;

            /// @brief 現在の文脈を評価してノード状態を返す。
            /// @param context  評価対象の文脈（読み書き可能）
            /// @return 評価結果
            virtual NodeStatus Tick(TContext& context) = 0;
        };

        template <typename TContext>
        using NodePtr = std::shared_ptr<Node<TContext>>;

        //=====================================================================
        // ConditionNode<TContext>  ― 条件判定ノード（葉）
        //
        // predicate が true を返せば Success、false なら Failure。
        // ゲームロジック上の「〜か？」をすべてここで表現する。
        //=====================================================================
        template <typename TContext>
        class ConditionNode : public Node<TContext>
        {
        public:
            /// @brief 条件判定関数を保持して条件ノードを構築する。
            /// @param predicate  文脈を受け取って bool を返す関数
            explicit ConditionNode(std::function<bool(const TContext&)> predicate)
                : m_predicate(std::move(predicate))
            {}

            /// @brief 条件式を評価し、成功か失敗を返す。
            NodeStatus Tick(TContext& context) override
            {
                if (!m_predicate) { return NodeStatus::Failure; }
                return m_predicate(context) ? NodeStatus::Success : NodeStatus::Failure;
            }

        private:
            std::function<bool(const TContext&)> m_predicate; ///< 条件判定関数
        };

        //=====================================================================
        // ActionNode<TContext>  ― アクション実行ノード（葉）
        //
        // action を呼び出してその結果をそのまま返す。
        // ゲームロジック上の「〜する」をすべてここで表現する。
        //=====================================================================
        template <typename TContext>
        class ActionNode : public Node<TContext>
        {
        public:
            /// @brief 実行関数を保持してアクションノードを構築する。
            /// @param action  文脈を受け取って NodeStatus を返す関数
            explicit ActionNode(std::function<NodeStatus(TContext&)> action)
                : m_action(std::move(action))
            {}

            /// @brief 実行関数を呼び出して結果状態を返す。
            NodeStatus Tick(TContext& context) override
            {
                if (!m_action) { return NodeStatus::Failure; }
                return m_action(context);
            }

        private:
            std::function<NodeStatus(TContext&)> m_action; ///< 実行関数
        };

        //=====================================================================
        // SequenceNode<TContext>  ― 順次評価ノード (AND)
        //
        // 子ノードを順番に評価し、最初の Failure/Running でそれを返す。
        // 全子ノードが Success なら Success を返す。
        // 「A かつ B かつ C ならば D」という条件チェーンに使う。
        //=====================================================================
        template <typename TContext>
        class SequenceNode : public Node<TContext>
        {
        public:
            /// @brief 順番に評価する子ノードを末尾に追加する。
            /// @param child  追加する子ノード（nullptr は無視）
            void AddChild(const NodePtr<TContext>& child)
            {
                if (child) { m_children.push_back(child); }
            }

            /// @brief 子ノードを順番に評価し、最初の失敗か実行中を返す。
            NodeStatus Tick(TContext& context) override
            {
                for (auto& child : m_children)
                {
                    const NodeStatus status = child->Tick(context);
                    if (status != NodeStatus::Success) { return status; }
                }
                return NodeStatus::Success;
            }

        private:
            std::vector<NodePtr<TContext>> m_children; ///< 子ノードリスト
        };

        //=====================================================================
        // SelectorNode<TContext>  ― 選択評価ノード (OR)
        //
        // 子ノードを順番に評価し、最初の Success/Running でそれを返す。
        // 全子ノードが Failure なら Failure を返す。
        // 「A か B か C」という優先順位付き選択に使う。
        //=====================================================================
        template <typename TContext>
        class SelectorNode : public Node<TContext>
        {
        public:
            /// @brief 候補として評価する子ノードを末尾に追加する。
            /// @param child  追加する子ノード（nullptr は無視）
            void AddChild(const NodePtr<TContext>& child)
            {
                if (child) { m_children.push_back(child); }
            }

            /// @brief 子ノードを順番に評価し、最初の成功か実行中を返す。
            NodeStatus Tick(TContext& context) override
            {
                for (auto& child : m_children)
                {
                    const NodeStatus status = child->Tick(context);
                    if (status == NodeStatus::Success || status == NodeStatus::Running)
                    {
                        return status;
                    }
                }
                return NodeStatus::Failure;
            }

        private:
            std::vector<NodePtr<TContext>> m_children; ///< 子ノードリスト
        };

    } // namespace AI
} // namespace Core
