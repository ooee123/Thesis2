package pdg;

import ast.BlockItem;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNode {
    @NonNull private BlockItem blockItem;
    private Collection<PDGNode> isADependencyFor;
    private Collection<PDGNode> dependsOn;
    private boolean required;

    public PDGNode(BlockItem blockItem) {
        this(blockItem, false);
    }

    public PDGNode(BlockItem blockItem, boolean required) {
        this.blockItem = blockItem;
        this.isADependencyFor = new ArrayList<>();
        this.dependsOn = new ArrayList<>();
        this.required = required;
    }
}
