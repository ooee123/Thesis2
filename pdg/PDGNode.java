package pdg;

import ast.BlockItem;
import ast.Statement;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

import java.util.IdentityHashMap;
import java.util.Map;

/**
 * Created by ooee on 10/9/16.
 */
@Data
@AllArgsConstructor
public class PDGNode {

    private static Map<BlockItem, PDGNode> allNodes = new IdentityHashMap<>();

    @NonNull private BlockItem blockItem;
    private PDGNode isADependencyFor;
    private PDGNode dependsOn;

    public static void link(BlockItem from, BlockItem to) {
        if (!allNodes.containsKey(from)) {
            allNodes.put(from, new PDGNode(from, null, null));
        }
        if (!allNodes.containsKey(to)) {
            allNodes.put(to, new PDGNode(to, null, null));
        }
        PDGNode fromNode = allNodes.get(from), toNode = allNodes.get(to);
        fromNode.setIsADependencyFor(toNode);
        toNode.setDependsOn(fromNode);
    }
}
