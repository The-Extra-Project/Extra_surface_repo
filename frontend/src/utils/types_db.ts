export type Json =
  | string
  | number
  | boolean
  | null
  | { [key: string]: Json | undefined }
  | Json[]

export type Database = {
  public: {
    Tables: {
      extra_surface: {
        Row: {
          email: string
          job_history:
            | Database["public"]["Tables"]["selectionjob"]["Row"][]
            | null
        }
        Insert: {
          email: string
          job_history?:
            | Database["public"]["Tables"]["selectionjob"]["Row"][]
            | null
        }
        Update: {
          email?: string
          job_history?:
            | Database["public"]["Tables"]["selectionjob"]["Row"][]
            | null
        }
        Relationships: []
      }
      payment: {
        Row: {
          email: string
          paid: boolean | null
          payment_id: string
          selection_job_id: string | null
        }
        Insert: {
          email: string
          paid?: boolean | null
          payment_id: string
          selection_job_id?: string | null
        }
        Update: {
          email?: string
          paid?: boolean | null
          payment_id?: string
          selection_job_id?: string | null
        }
        Relationships: [
          {
            foreignKeyName: "payment_selection_job_id_fkey"
            columns: ["selection_job_id"]
            isOneToOne: false
            referencedRelation: "selectionjob"
            referencedColumns: ["job_id"]
          },
        ]
      }
      reconstructed_tiles: {
        Row: {
          ipfs_dir: string | null
          lasttime_updated: string | null
          tile_name: string
        }
        Insert: {
          ipfs_dir?: string | null
          lasttime_updated?: string | null
          tile_name: string
        }
        Update: {
          ipfs_dir?: string | null
          lasttime_updated?: string | null
          tile_name?: string
        }
        Relationships: []
      }
      selectionjob: {
        Row: {
          geocordinate_copc: string[] | null
          job_created_at: string | null
          job_id: string
          status: boolean | null
          upload_url_file: string
        }
        Insert: {
          geocordinate_copc?: string[] | null
          job_created_at?: string | null
          job_id: string
          status?: boolean | null
          upload_url_file: string
        }
        Update: {
          geocordinate_copc?: string[] | null
          job_created_at?: string | null
          job_id?: string
          status?: boolean | null
          upload_url_file?: string
        }
        Relationships: []
      }
    }
    Views: {
      [_ in never]: never
    }
    Functions: {
      [_ in never]: never
    }
    Enums: {
      [_ in never]: never
    }
    CompositeTypes: {
      [_ in never]: never
    }
  }
}

type PublicSchema = Database[Extract<keyof Database, "public">]

export type Tables<
  PublicTableNameOrOptions extends
    | keyof (PublicSchema["Tables"] & PublicSchema["Views"])
    | { schema: keyof Database },
  TableName extends PublicTableNameOrOptions extends { schema: keyof Database }
    ? keyof (Database[PublicTableNameOrOptions["schema"]]["Tables"] &
        Database[PublicTableNameOrOptions["schema"]]["Views"])
    : never = never,
> = PublicTableNameOrOptions extends { schema: keyof Database }
  ? (Database[PublicTableNameOrOptions["schema"]]["Tables"] &
      Database[PublicTableNameOrOptions["schema"]]["Views"])[TableName] extends {
      Row: infer R
    }
    ? R
    : never
  : PublicTableNameOrOptions extends keyof (PublicSchema["Tables"] &
        PublicSchema["Views"])
    ? (PublicSchema["Tables"] &
        PublicSchema["Views"])[PublicTableNameOrOptions] extends {
        Row: infer R
      }
      ? R
      : never
    : never

export type TablesInsert<
  PublicTableNameOrOptions extends
    | keyof PublicSchema["Tables"]
    | { schema: keyof Database },
  TableName extends PublicTableNameOrOptions extends { schema: keyof Database }
    ? keyof Database[PublicTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = PublicTableNameOrOptions extends { schema: keyof Database }
  ? Database[PublicTableNameOrOptions["schema"]]["Tables"][TableName] extends {
      Insert: infer I
    }
    ? I
    : never
  : PublicTableNameOrOptions extends keyof PublicSchema["Tables"]
    ? PublicSchema["Tables"][PublicTableNameOrOptions] extends {
        Insert: infer I
      }
      ? I
      : never
    : never

export type TablesUpdate<
  PublicTableNameOrOptions extends
    | keyof PublicSchema["Tables"]
    | { schema: keyof Database },
  TableName extends PublicTableNameOrOptions extends { schema: keyof Database }
    ? keyof Database[PublicTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = PublicTableNameOrOptions extends { schema: keyof Database }
  ? Database[PublicTableNameOrOptions["schema"]]["Tables"][TableName] extends {
      Update: infer U
    }
    ? U
    : never
  : PublicTableNameOrOptions extends keyof PublicSchema["Tables"]
    ? PublicSchema["Tables"][PublicTableNameOrOptions] extends {
        Update: infer U
      }
      ? U
      : never
    : never

export type Enums<
  PublicEnumNameOrOptions extends
    | keyof PublicSchema["Enums"]
    | { schema: keyof Database },
  EnumName extends PublicEnumNameOrOptions extends { schema: keyof Database }
    ? keyof Database[PublicEnumNameOrOptions["schema"]]["Enums"]
    : never = never,
> = PublicEnumNameOrOptions extends { schema: keyof Database }
  ? Database[PublicEnumNameOrOptions["schema"]]["Enums"][EnumName]
  : PublicEnumNameOrOptions extends keyof PublicSchema["Enums"]
    ? PublicSchema["Enums"][PublicEnumNameOrOptions]
    : never
