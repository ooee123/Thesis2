package pdg;

import ast.BlockItem;
import lombok.Getter;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */

public abstract class PDGNode<T extends BlockItem> {
    @NonNull T blockItem;
    @Getter Collection<PDGNode<? extends BlockItem>> isADependencyFor;
    @Getter Collection<PDGNode<? extends BlockItem>> dependsOn;
    @Getter boolean required;

    public PDGNode(T blockItem) {
        this(blockItem, false);
    }

    public PDGNode(T blockItem, boolean required) {
        this.blockItem = blockItem;
        this.isADependencyFor = new ArrayList<>();
        this.dependsOn = new ArrayList<>();
        this.required = required;
    }

    public abstract T sort(PDGSorter sorter);
}
