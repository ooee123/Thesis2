package ast;

import com.google.common.collect.Lists;
import lombok.Value;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class CompoundStatement implements Statement {
    private List<BlockItem> blockItems;

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        int number = 1;
        for (BlockItem blockItem : blockItems) {
            builder.append("\n");
            builder.append(number);
            builder.append(": ");
            builder.append(blockItem.toString());
            number++;
        }
        builder.append("\n");
        return builder.toString();
    }

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(blockItems);
        return "{\n " + String.join("\n", codes) + "\n}";
    }
}
