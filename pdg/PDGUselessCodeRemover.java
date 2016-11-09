package pdg;

import ast.BlockItem;
import ast.CompoundStatement;
import ast.Statement;
import com.google.common.collect.Sets;
import lombok.Setter;
import visitor.PDGGenerationVisitor;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 10/21/16.
 */
public class PDGUselessCodeRemover {

    public static final boolean TREAT_MEMORY_WRITES_AS_CRITICAL_AUTOMATICALLY = true;
    private Set<PDGNode<? extends BlockItem>> visited;

    public PDGUselessCodeRemover() {
        visited = Sets.newIdentityHashSet();
    }

    public void removeUselessCode(PDGNodeCompoundStatement node) {
        Collection<PDGNode<? extends BlockItem>> nodes = node.getBody();
        markRequiredNodes(node);
        removeAllNonRequiredNodes(nodes);
    }

    private void markAsRequired(PDGNodeCompoundStatement pdgNode) {
        for (PDGNode<? extends BlockItem> node : pdgNode.getBody()) {
            if (node.isRequired() && !visited.contains(node)) {
                propagateRequired(node);
            }
        }
    }

    /**
     *
     * @param node
     */
    private void propagateRequired(PDGNode<? extends BlockItem> node) {
        for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
            if (!visited.contains(pdgNode)) {
                visited.add(pdgNode);
                pdgNode.required = true;
                propagateRequired(pdgNode);
            }
            if (pdgNode instanceof PDGNodeContainsStatementNode) {
                propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) pdgNode), node.getBlockItem().getDependantVariables());
            }
        }
    }

    private void propagateThroughStatementNodes(PDGNodeContainsStatementNode<? extends Statement> node, Set<String> variablesToBeRequired) {
        for (PDGNode<? extends Statement> pdgNode : node.getStatementNodes()) {
            if (pdgNode instanceof PDGNodeCompoundStatement) {
                propagateRequiredCompoundStatement(variablesToBeRequired, (PDGNodeCompoundStatement)pdgNode);
            } else if (pdgNode instanceof PDGNodeContainsStatementNode) {
                propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) pdgNode), variablesToBeRequired);
            }
        }
    }

    /**
     * Takes every String in dependVariables and marks the BlockItem inside pdgNode (A compound statement), as critical.
     * @param dependVariables
     * @param pdgNode
     */
    public void propagateRequiredCompoundStatement(Set<String> dependVariables, PDGNodeCompoundStatement pdgNode) {
        for (String dependVariable : dependVariables) {
            if (pdgNode.getLastAssigned().containsKey(dependVariable)) {
                for (Collection<PDGNode<? extends BlockItem>> nodes : pdgNode.getLastAssigned().getAllAssociated(dependVariable)) {
                    for (PDGNode<? extends BlockItem> node : nodes) {
                        if (!visited.contains(node)) {
                            node.required = true;
                            visited.add(node);
                            propagateRequired(node);
                        }
                        if (node instanceof PDGNodeContainsStatementNode) {
                            Set<String> newSet = Sets.newIdentityHashSet();
                            newSet.add(dependVariable);
                            propagateThroughStatementNodes(((PDGNodeContainsStatementNode<? extends Statement>) node), newSet);
                        }
                    }
                }
            }
        }
    }

    /**
     * Starting point
     */
    private void markRequiredNodes(PDGNodeCompoundStatement pdgNodeCompoundStatement) {
        Collection<PDGNode<? extends BlockItem>> nodes = pdgNodeCompoundStatement.getBody();
        for (PDGNode<? extends BlockItem> node : nodes) {
            if ((node.blockItem.isCritical() || node.isRequired()) && !visited.contains(node)) {
                node.required = true;
                visited.add(node);
                propagateRequired(node);
            }
            if (node instanceof PDGNodeContainsStatementNode) {
                Collection<PDGNode<? extends Statement>> statementNodes = ((PDGNodeContainsStatementNode) node).getStatementNodes();
                for (PDGNode<? extends Statement> statementNode : statementNodes) {
                    if (statementNode instanceof PDGNodeCompoundStatement) {
                        markRequiredNodes(((PDGNodeCompoundStatement) statementNode));
                    }
                }
            }
            if (node instanceof PDGNodeCompoundStatement) {
                markRequiredNodes(((PDGNodeCompoundStatement) node));
            }
        }
    }

    private void removeAllNonRequiredNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Set<PDGNode<? extends BlockItem>> notRequired = Sets.newIdentityHashSet();
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
        for (PDGNode<? extends BlockItem> pdgNode : notRequired) {
            PDGNode.removeNode(pdgNode, nodes);
        }
    }
/*
    private static void removeEmptyCompoundStatements(Collection<PDGNode<? extends BlockItem>> nodes) {
        for (PDGNode<? extends BlockItem> node : nodes) {
            node instanceof
        }
    }
    */
}
