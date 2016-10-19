package pdg;

import ast.BlockItem;
import ast.CompoundStatement;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeCompoundStatement extends PDGNode<CompoundStatement> {
    @NonNull private CompoundStatement compoundStatement;
    private Collection<PDGNode> body;
    private Collection<PDGNode> isADependencyFor;
    private Collection<PDGNode> dependsOn;
    private boolean required;

    public PDGNodeCompoundStatement(BlockItem blockItem) {
        this(blockItem, false);
    }

    public PDGNodeCompoundStatement(BlockItem blockItem, boolean required) {
        this.blockItem = blockItem;
        this.isADependencyFor = new ArrayList<>();
        this.dependsOn = new ArrayList<>();
        this.required = required;
    }

    public String toCode(PDGSorterDefault sorter) {
    }
}
