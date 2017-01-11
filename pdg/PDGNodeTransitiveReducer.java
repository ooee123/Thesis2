package pdg;

import ast.BlockItem;

import java.util.*;

/**
 * Created by ooee on 10/24/16.
 */
public class PDGNodeTransitiveReducer {
    public void reduce(Collection<PDGNode<? extends BlockItem>> nodes) {
        Map<PDGNode<? extends BlockItem>, Boolean> visited = new IdentityHashMap<>();
        for (PDGNode<? extends BlockItem> node : nodes) {
            processOneNode(node);
        }
    }

    public void reduce(PDGNode<? extends BlockItem> node) {
        if (node instanceof PDGNodeCompoundStatement) {
            reduce(((PDGNodeCompoundStatement) node).getBody());
        } else if (node instanceof PDGNodeFor) {
            reduce(((PDGNodeFor) node).getBody());
        } else if (node instanceof PDGNodeDeclareFor) {
            reduce(((PDGNodeDeclareFor) node).getBody());
        } else if (node instanceof PDGNodeWhile) {
            reduce(((PDGNodeWhile) node).getBody());
        } else if (node instanceof PDGNodeDoWhile) {
            reduce(((PDGNodeDoWhile) node).getBody());
        } else if (node instanceof PDGNodeIf) {
            reduce(((PDGNodeIf) node).getThenNode());
            if (((PDGNodeIf) node).getElseNode() != null) {
                reduce(((PDGNodeIf) node).getElseNode());
            }
        }
    }

    public void reduce(PDGNodeCompoundStatement node) {
        reduce(node.getBody());
    }

    private void processOneNode(PDGNode<? extends BlockItem> node) {
        if (node.getIsADependencyFor().size() > 1) {
            Collection<PDGNode<? extends BlockItem>> visited = new HashSet<>();
            for (PDGNode<? extends BlockItem> pdgNode : node.getIsADependencyFor()) {
                depthFirstSearch(pdgNode, visited);
            }
            for (PDGNode<? extends BlockItem> pdgNode : visited) {
                if (node.getIsADependencyFor().contains(pdgNode)) {
                    unlink(node, pdgNode);
                }
            }
        }
        if (node.getIsAheadOfMe().size() > 1) {
            Collection<PDGNode<? extends BlockItem>> visited = new HashSet<>();
            for (PDGNode<? extends BlockItem> pdgNode : node.getIsAheadOfMe()) {
                depthFirstSearchBehind(pdgNode, visited);
            }
            for (PDGNode<? extends BlockItem> pdgNode : visited) {
                if (node.getIsAheadOfMe().contains(pdgNode)) {
                    unlinkBehind(node, pdgNode);
                }
            }
        }
        reduce(node);
    }

    private Collection<PDGNode<? extends BlockItem>> depthFirstSearch(PDGNode<? extends BlockItem> node, Collection<PDGNode<? extends BlockItem>> visited) {
        for (PDGNode<? extends BlockItem> pdgNode : node.getIsADependencyFor()) {
            if (!visited.contains(pdgNode)) {
                visited.add(pdgNode);
                depthFirstSearch(pdgNode, visited);
            }
        }
        return visited;
    }

    private Collection<PDGNode<? extends BlockItem>> depthFirstSearchBehind(PDGNode<? extends BlockItem> node, Collection<PDGNode<? extends BlockItem>> visited) {
        for (PDGNode<? extends BlockItem> pdgNode : node.getIsAheadOfMe()) {
            if (!visited.contains(pdgNode)) {
                visited.add(pdgNode);
                depthFirstSearchBehind(pdgNode, visited);
            }
        }
        return visited;
    }

    private void unlink(PDGNode<? extends BlockItem> hostNode, PDGNode<? extends BlockItem> otherNode) {
        hostNode.getIsADependencyFor().remove(otherNode);
        otherNode.getDependsOn().remove(hostNode);
    }

    private void unlinkBehind(PDGNode<? extends BlockItem> hostNode, PDGNode<? extends BlockItem> otherNode) {
        hostNode.getIsAheadOfMe().remove(otherNode);
        otherNode.getIsBehindOfMe().remove(hostNode);
    }
}
