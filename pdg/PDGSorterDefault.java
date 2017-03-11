package pdg;

import ast.*;
import ast.declaration.TypedefDeclaration;
import ast.declaration.VariableDeclaration;
import ast.statement.impl.CompoundStatement;
import ast.statement.JumpStatementStrict;
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
        Collection<TypedefDeclaration> typedefs = new ArrayList<>();
        Collection<VariableDeclaration> emptyVariableDeclarations = Sets.newIdentityHashSet();
        Collection<PDGNode<? extends BlockItem>> emptyDeclarationNodes = Sets.newIdentityHashSet();
        for (PDGNode node : nodes) {
            if (node.blockItem instanceof VariableDeclaration) {
                if (node.blockItem.getDependantVariables().isEmpty()) {
                    emptyVariableDeclarations.add(((VariableDeclaration) node.blockItem));
                    emptyDeclarationNodes.add(node);
                }
            }
            if (node.blockItem instanceof TypedefDeclaration) {
                typedefs.add(((TypedefDeclaration) node.blockItem));
            }
        }
        blockItems.addAll(typedefs);
        nodes.removeAll(typedefs);

        while(!emptyDeclarationNodes.isEmpty()) {
            PDGNode<? extends BlockItem> nextNode = pickNextNode(emptyDeclarationNodes);
            PDGNode.removeNode(nextNode, nodes);
            emptyDeclarationNodes.remove(nextNode);
            blockItems.add(nextNode.blockItem);

        }
        /*
        blockItems.addAll(emptyVariableDeclarations);
        for (PDGNode emptyDeclarationNode : emptyDeclarationNodes) {
            PDGNode.removeNode(emptyDeclarationNode, nodes);
        }
        */

        while (nodes.size() > 0) {
            Collection<PDGNode<? extends BlockItem>> candidateNextNodes = getReadyNodes(nodes);
            PDGNode chosenNode = pickNextNode(candidateNextNodes);
            PDGNode.removeNode(chosenNode, nodes);
            blockItems.add(processNode(chosenNode));
        }
        return new CompoundStatement(blockItems);
    }

    private PDGNode<? extends BlockItem> pickNextNode(Collection<PDGNode<? extends BlockItem>> candidates) {
        return Collections.min(candidates, new PDGNodeComparator());
    }

    private BlockItem processNode(PDGNode node) {
        return node.sort(this);
    }

    private Collection<PDGNode<? extends BlockItem>> getReadyNodes(Collection<PDGNode<? extends BlockItem>> nodes) {
        Collection<PDGNode<? extends BlockItem>> readyNodes = Sets.newIdentityHashSet();

        for (PDGNode<? extends BlockItem> node : nodes) {
            if (node.getDependsOn().isEmpty() && node.getIsBehindOfMe().isEmpty()) {
                if (!(node.getBlockItem() instanceof JumpStatementStrict) || nodes.size() <= 1) {
                    readyNodes.add(node);
                }
            }
        }
        if (readyNodes.size() == 0 && nodes.size() > 1) {
            if (nodes.stream().allMatch(node -> node.blockItem instanceof JumpStatementStrict)) {
                readyNodes.add(nodes.stream().findFirst().get());
            }
        }
        return readyNodes;
    }
}