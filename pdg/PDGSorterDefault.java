package pdg;

import ast.BlockItem;
import ast.CompoundStatement;
import ast.SelectionStatementIf;
import ast.Statement;
import lombok.Value;

import java.rmi.activation.ActivationGroup_Stub;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 10/14/16.
 */
@Value
public class PDGSorterDefault implements PDGSorter {

    /**
     *
     * @param nodes
     * @return
     */
    @Override
    public Statement sort(Collection<PDGNode> nodes) {
        List<BlockItem> blockItems = new ArrayList<>();
        //processNonPrerequisiteNodes(nodes);
        while (nodes.size() > 0) {
            System.out.println(nodes.size());
            Collection<PDGNode> candidateNextNodes = getReadyNodes(nodes);
            System.out.println(candidateNextNodes.size());
            PDGNode chosenNode = pickNextNode(candidateNextNodes);
            removeNode(chosenNode, nodes);
            blockItems.add(processNode(chosenNode));
        }
        if (blockItems.size() > 1) {
            return new CompoundStatement(blockItems);
        } else {
            return ((Statement) blockItems.get(0));
        }
    }

    public Set<PDGNode> getPDGForest(PDGNode node, Set<PDGNode> forest) {
        for (Object obj : node.getDependsOn()) {
            PDGNode dependsOn = (PDGNode) obj;
            if (!forest.contains(dependsOn)) {
                forest.add(dependsOn);
                getPDGForest(dependsOn, forest);
            }
        }
        for (Object obj : node.getIsADependencyFor()) {
            PDGNode aDependencyFor = (PDGNode) obj;
            if (!forest.contains(aDependencyFor)) {
                forest.add(aDependencyFor);
                getPDGForest(aDependencyFor, forest);
            }
        }
        return forest;
    }

    private PDGNode pickNextNode(Collection<PDGNode> candidates) {
        int mostDependsOn = -1;
        PDGNode best;
        best = candidates.stream().findFirst().get();
        for (PDGNode candidate : candidates) {
            if (candidate.getIsADependencyFor().size() > mostDependsOn) {
                best = candidate;
                mostDependsOn = candidate.getIsADependencyFor().size();
            }
        }

        return best;
    }

    private void removeNode(PDGNode node, Collection<PDGNode> nodes) {
        nodes.remove(node);
        Collection<PDGNode> dependents = node.getIsADependencyFor();
        for (PDGNode dependent : dependents) {
            dependent.getDependsOn().remove(node);
        }
    }

    private BlockItem processNode(PDGNode node) {
        BlockItem blockItem = node.sort(this);
        return blockItem;
    }

    private Collection<PDGNode> getReadyNodes(Collection<PDGNode> nodes) {
        Collection<PDGNode> readyNodes = new ArrayList<>();
        for (PDGNode node : nodes) {
            if (node.getDependsOn().isEmpty()) {
                readyNodes.add(node);
            }
        }
        return readyNodes;
    }

    private Collection<PDGNode> processNonPrerequisiteNodes(Collection<PDGNode> nodes) {
        int size = 0;
        Collection<PDGNode> nonPreNodes = new ArrayList<>();

        for (PDGNode node : nodes) {
            if (node.getIsADependencyFor().isEmpty() && !node.isRequired()) {
                nonPreNodes.add(node);
            }
        }
        nodes.removeAll(nonPreNodes);
        while (nonPreNodes.size() != size) {
            size = nonPreNodes.size();
            for (PDGNode node : nodes) {
                if (node.getIsADependencyFor().isEmpty() && !node.isRequired()) {
                    nonPreNodes.add(node);
                }
            }
            nodes.removeAll(nonPreNodes);
        }
        return nonPreNodes;
    }
}
