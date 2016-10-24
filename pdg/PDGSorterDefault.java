package pdg;

import ast.*;
import com.google.common.collect.Sets;
import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 10/14/16.
 */
@Value
public class PDGSorterDefault implements PDGSorter {

    @Override
    public CompoundStatement sort(Collection<PDGNode<? extends BlockItem>> nodes) {
        List<BlockItem> blockItems = new ArrayList<>();
        List<Declaration> emptyDeclarations = new ArrayList<>();
        List<PDGNode> emptyDeclarationNodes = new ArrayList<>();
        for (PDGNode node : nodes) {
            if (node.blockItem instanceof Declaration) {
                if (node.blockItem.getDependantVariables().isEmpty()) {
                    emptyDeclarations.add(((Declaration) node.blockItem));
                    emptyDeclarationNodes.add(node);
                }
            }
        }
        blockItems.addAll(emptyDeclarations);
        for (PDGNode emptyDeclarationNode : emptyDeclarationNodes) {
            removeNode(emptyDeclarationNode, nodes);
        }

        while (nodes.size() > 0) {
            Collection<PDGNode<? extends BlockItem>> candidateNextNodes = getReadyNodes(nodes);
            PDGNode chosenNode = pickNextNode(candidateNextNodes);
            removeNode(chosenNode, nodes);
            blockItems.add(processNode(chosenNode));
        }
        return new CompoundStatement(blockItems);
    }

    private PDGNode<? extends BlockItem> pickNextNode(Collection<PDGNode<? extends BlockItem>> candidates) {
        int mostDependsOn = -1;
        PDGNode best;
        best = candidates.stream().findFirst().get();
        for (PDGNode candidate : candidates) {
            if (!(candidate.blockItem instanceof JumpStatementStrict) && candidate.getIsADependencyFor().size() > mostDependsOn) {
                best = candidate;
                mostDependsOn = candidate.getIsADependencyFor().size();
            }
        }
        return best;
    }

    private void removeNode(PDGNode<? extends BlockItem> node, Collection<PDGNode<? extends BlockItem>> nodes) {
        nodes.remove(node);
        Collection<PDGNode<? extends BlockItem>> dependents = node.getIsADependencyFor();
        for (PDGNode dependent : dependents) {
            dependent.getDependsOn().remove(node);
        }
    }

    private BlockItem processNode(PDGNode node) {
        return node.sort(this);
    }

    private Collection<PDGNode<? extends BlockItem>> getReadyNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Collection<PDGNode<? extends BlockItem>> readyNodes = new ArrayList<>();
        for (PDGNode<? extends BlockItem> node : nodes) {
            if (node.getDependsOn().isEmpty()) {
                readyNodes.add(node);
            }
        }
        return readyNodes;
    }
}