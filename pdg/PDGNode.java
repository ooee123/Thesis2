package pdg;

import ast.BlockItem;
import lombok.NonNull;
import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNode {
    @NonNull private BlockItem blockItem;
    private Collection<PDGNode> isADependencyFor;
    private Collection<PDGNode> dependsOn;
}
