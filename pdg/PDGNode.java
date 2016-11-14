package pdg;

import ast.BlockItem;
import ast.Statement;
import lombok.*;
import visitor.PDGGenerationVisitor;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;

/**
 * Created by ooee on 10/9/16.
 */

public abstract class PDGNode<T extends BlockItem> {
    @Getter @NonNull T blockItem;
    // Nodes that must be behind for variable dependencies
    @Getter Collection<PDGNode<? extends BlockItem>> isADependencyFor;
    // Nodes that I depend on for their variables
    @Getter Collection<PDGNode<? extends BlockItem>> dependsOn;
    // Nodes that are behind this node. (This node is ahead of these nodes)
    @Getter Collection<PDGNode<? extends BlockItem>> isBehindOfMe;
    // Nodes that are ahead of this node. (This node is behind these nodes)
    @Getter Collection<PDGNode<? extends BlockItem>> isAheadOfMe;
    @Getter @Setter boolean required;

    public PDGNode(T blockItem) {
        this(blockItem, false);
    }

    public PDGNode(T blockItem, boolean required) {
        this.blockItem = blockItem;
        this.isADependencyFor = new HashSet<>();
        this.dependsOn = new HashSet<>();
        this.isAheadOfMe = new HashSet<>();
        this.isBehindOfMe = new HashSet<>();
        this.required = required;
    }

    public abstract T sort(PDGSorter sorter);

    @Override
    public boolean equals(Object o) {
        if (o != null) {
            return false;
        }
        if (o instanceof PDGNode) {
            return blockItem == ((PDGNode) o).blockItem;
        }
        return false;
    }

    public void linkVariableDependency(PDGNode toNode) {
        if (this != toNode) {
            this.getIsADependencyFor().add(toNode);
            toNode.getDependsOn().add(this);
        }
    }

    public void linkOrderDependency(PDGNode toNode) {
        if (this != toNode) {
            this.getIsAheadOfMe().add(toNode);
            toNode.getIsBehindOfMe().add(this);
        }
    }

    public static void removeNode(PDGNode<? extends BlockItem> node, Collection<PDGNode<? extends BlockItem>> nodes) {
        nodes.remove(node);
        for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
            pdgNode.getIsADependencyFor().remove(node);
            for (PDGNode<? extends BlockItem> pdgNode1 : node.getIsADependencyFor()) {
                pdgNode.linkOrderDependency(pdgNode1);
            }
        }
        for (PDGNode dependent : node.getIsADependencyFor()) {
            dependent.getDependsOn().remove(node);
            /*
            for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
                pdgNode.linkOrderDependency(dependent);
            }
            */
        }
        for (PDGNode<? extends BlockItem> pdgNode : node.getIsBehindOfMe()) {
            pdgNode.getIsAheadOfMe().remove(node);
            for (PDGNode<? extends BlockItem> pdgNode1 : node.getIsAheadOfMe()) {
                pdgNode.linkOrderDependency(pdgNode1);
            }
        }
        for (PDGNode aheadOfMe : node.getIsAheadOfMe()) {
            aheadOfMe.getIsBehindOfMe().remove(node);
            /*
            for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
                pdgNode.linkOrderDependency(aheadOfMe);
            }
            */
        }
    }
/*
    public static void removeNode(PDGNode<? extends BlockItem> node, Collection<PDGNode<? extends BlockItem>> nodes) {
        nodes.remove(node);
        for (PDGNode<? extends BlockItem> pdgNode : node.getDependsOn()) {
            pdgNode.getIsADependencyFor().remove(node);
        }
        for (PDGNode<? extends BlockItem> pdgNode : node.getIsBehindOfMe()) {
            pdgNode.getIsAheadOfMe().remove(node);
        }
        for (PDGNode dependent : node.getIsADependencyFor()) {
            dependent.getDependsOn().remove(node);
        }
        for (PDGNode aheadOfMe : node.getIsAheadOfMe()) {
            aheadOfMe.getDependsOn().remove(node);
        }
    }
*/
}
