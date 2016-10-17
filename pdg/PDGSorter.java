package pdg;

import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;
import java.util.stream.Collectors;

/**
 * Created by ooee on 10/14/16.
 */
@Value
public class PDGSorter {
    private Collection<PDGNode> nodes;

    public void process() {
        processNonPrerequisiteNodes();
        int previousSize = 0;
        while (previousSize != nodes.size()) {
            processNode(pickNextNode(getReadyNodes()));
        }
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

    private void processNode(PDGNode node) {
        nodes.remove(node);
        Collection<PDGNode> dependents = node.getIsADependencyFor();
        for (PDGNode dependent : dependents) {
            dependent.getDependsOn().remove(node);
        }
        System.out.println(node.getBlockItem().toCode());
    }

    private Collection<PDGNode> getReadyNodes() {
        Collection<PDGNode> readyNodes = new ArrayList<>();
        for (PDGNode node : nodes) {
            if (node.getDependsOn().isEmpty()) {
                readyNodes.add(node);
            }
        }
        return readyNodes;
    }

    private Collection<PDGNode> processNonPrerequisiteNodes() {
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
