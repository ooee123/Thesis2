package pdg;

import ast.*;
import com.google.common.collect.Lists;
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
            PDGNode.removeNode(emptyDeclarationNode, nodes);
        }

        while (nodes.size() > 0) {
            Collection<PDGNode<? extends BlockItem>> candidateNextNodes = getReadyNodes(nodes);
            PDGNode chosenNode = pickNextNode(candidateNextNodes);
            PDGNode.removeNode(chosenNode, nodes);
            blockItems.add(processNode(chosenNode));
        }
        return new CompoundStatement(blockItems);
    }

    private PDGNode<? extends BlockItem> pickNextNode(Collection<PDGNode<? extends BlockItem>> candidates) {
        int mostDependsOn = -1;
        Collection<PDGNode<? extends BlockItem>> bests = new ArrayList<>();
        bests.add(candidates.stream().findFirst().get());
        for (PDGNode candidate : candidates) {
            if (!(candidate.blockItem instanceof JumpStatementStrict) && candidate.getIsADependencyFor().size() > mostDependsOn) {
                bests = Lists.newArrayList(candidate);
                mostDependsOn = candidate.getIsADependencyFor().size();
            } else if (!(candidate.blockItem instanceof JumpStatementStrict) && candidate.getIsADependencyFor().size() == mostDependsOn) {
                bests.add(candidate);
            }
        }
        if (bests.size() == 1) {
            return bests.stream().findAny().get();
        } else {
            return bests.stream().findAny().get();
            //return Collections.max(bests, new BlockItemComparator());
        }
    }

    private BlockItem processNode(PDGNode node) {
        return node.sort(this);
    }

    private Collection<PDGNode<? extends BlockItem>> getReadyNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Collection<PDGNode<? extends BlockItem>> readyNodes = new ArrayList<>();
        for (PDGNode<? extends BlockItem> node : nodes) {
            if (node.getDependsOn().isEmpty() && node.getIsBehindOfMe().isEmpty()) {
                readyNodes.add(node);
            }
        }
        return readyNodes;
    }
}