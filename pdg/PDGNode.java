package pdg;

import ast.BlockItem;
import ast.Statement;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;

/**
 * Created by ooee on 10/9/16.
 */

public abstract class PDGNode<T extends BlockItem> {
    @Getter @NonNull T blockItem;
    @Getter Collection<PDGNode<? extends BlockItem>> isADependencyFor;
    @Getter Collection<PDGNode<? extends BlockItem>> dependsOn;
    @Getter boolean required;

    public PDGNode(T blockItem) {
        this(blockItem, false);
    }

    public PDGNode(T blockItem, boolean required) {
        this.blockItem = blockItem;
        this.isADependencyFor = new HashSet<>();
        this.dependsOn = new HashSet<>();
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
}
