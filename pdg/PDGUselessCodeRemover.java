package pdg;

import ast.BlockItem;
import ast.Statement;
import com.google.common.collect.Sets;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 10/21/16.
 */
public class PDGUselessCodeRemover {

    public void removeUselessCode(Collection<PDGNode<? extends BlockItem>> nodes) {
        markRequiredNodes(nodes);
        removeAllNonRequiredNodes(nodes);
    }

    private void propagateRequired(PDGNode<? extends BlockItem> node, Set<PDGNode<? extends BlockItem>> required) {
        for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
            if (!required.contains(pdgNode)) {
                required.add(pdgNode);
                pdgNode.required = true;
                propagateRequired(pdgNode, required);
            }
            if (pdgNode instanceof PDGNodeContainsStatementNode) {
                propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) pdgNode), node.getBlockItem().getDependantVariables());
            }
        }
    }

    private void propagateThroughStatementNodes(PDGNodeContainsStatementNode<? extends Statement> node, Set<String> variablesToBeRequired) {
        for (PDGNode<? extends Statement> pdgNode : node.getStatementNodes()) {
            if (pdgNode instanceof PDGNodeCompoundStatement) {
                propagateRequiredCompoundStatement(variablesToBeRequired, (PDGNodeCompoundStatement)pdgNode, new HashSet<>());
            } else if (pdgNode instanceof PDGNodeContainsStatementNode) {
                propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) pdgNode), variablesToBeRequired);
            }
        }
    }

    private void propagateRequiredCompoundStatement(Set<String> dependVariables, PDGNodeCompoundStatement pdgNode, Set<PDGNode<? extends BlockItem>> required) {
        for (String dependVariable : dependVariables) {
            for (PDGNode<? extends BlockItem> node : pdgNode.getLastAssigned().get(dependVariable)) {
                if (!required.contains(node)) {
                    node.required = true;
                    required.add(node);
                    propagateRequired(node, required);
                }
                if (node instanceof PDGNodeContainsStatementNode) {
                    propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) node), Sets.newHashSet(dependVariable));
                }
            }
        }
    }

    private Collection<PDGNode<? extends BlockItem>> markRequiredNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Set<PDGNode<? extends BlockItem>> required = new HashSet<>();
        for (PDGNode<? extends BlockItem> node : nodes) {
            if (node.isRequired() && !required.contains(node)) {
                required.add(node);
                propagateRequired(node, required);
            }
        }
        return nodes;
    }

    private void removeAllNonRequiredNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Set<PDGNode<? extends BlockItem>> notRequired = new HashSet<>();
        for (PDGNode<? extends BlockItem> node : nodes) {
            if (node instanceof PDGNodeContainsStatementNode) {
                for (PDGNode<? extends Statement> pdgNode : ((PDGNodeContainsStatementNode<? extends Statement>) node).getStatementNodes()) {
                    if (pdgNode instanceof PDGNodeCompoundStatement) {
                        removeAllNonRequiredNodes(((PDGNodeCompoundStatement) pdgNode).getBody());
                    }
                }
            }
            if (!node.isRequired()) {
                notRequired.add(node);
            }
        }
        nodes.removeAll(notRequired);
    }
}
