package ast;

import lombok.Value;

import java.util.List;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class CompoundStatement implements Statement {
    private List<BlockItem> blockItems;
}
